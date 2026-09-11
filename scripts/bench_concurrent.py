#!/usr/bin/env python3
"""Concurrency benchmark: can the stack serve several coding requests at once?

Decode on Apple Silicon is bound by reading the weights out of memory rather than
by arithmetic, so a stack that batches properly can serve N requests for not much
more than the cost of one. Everything else in this repo measures a single request
at a time, which says nothing about an agent firing off parallel tool calls or a
team sharing one server.

The workload is fixed: the same 8 C tasks every time, run with 1, 2, 4 and 8
requests in flight. Identical work at every level, so wall times compare directly.
Every answer is compiled and tested, because if accuracy drops as concurrency
rises that is a KV-cache bug in the serving stack and worth catching.

Results are written to results/concurrency/<target>-<timestamp>.json and are never
overwritten, so runs can be compared over time.
"""
from __future__ import annotations

import argparse
import datetime as dt
import json
import statistics
import sys
import tempfile
import threading
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import Any

import httpx

sys.path.insert(0, str(Path(__file__).resolve().parent))

import eval_code
from bench import TARGETS, RssSampler, _openai_pids

RESULTS = Path(__file__).resolve().parent.parent / "results" / "concurrency"
LEVELS = (1, 2, 4, 8, 12, 16)
WORKLOAD = 8  # requests per level, identical at every level


def build_prompts() -> list[dict[str, str]]:
    """Eight verified C tasks — known solvable, graded deterministically."""
    tasks = eval_code.task_set("easy")[:WORKLOAD]
    if len(tasks) < WORKLOAD:
        raise SystemExit(f"need {WORKLOAD} tasks, only found {len(tasks)}")
    return tasks


def prompt_for(task: dict[str, str]) -> str:
    return (
        f"Implement in C11: `{task['sig']}`. {task['prompt']}\n"
        "Reply with only a C code block. No main function, no tests, no explanation."
    )


def one_request(*, port: int, model: str, prompt: str, max_tokens: int,
                timeout: float) -> dict[str, Any]:
    """One streaming completion, reporting time to first token and decode rate."""
    body = {
        "model": model,
        "messages": [{"role": "user", "content": prompt}],
        "max_tokens": max_tokens,
        "temperature": 0,
        "stream": True,
        "stream_options": {"include_usage": True},
    }
    url = f"http://127.0.0.1:{port}/v1/chat/completions"
    parts: list[str] = []
    ttft: float | None = None
    completion_tokens: int | None = None
    start = time.perf_counter()
    try:
        with httpx.Client(timeout=httpx.Timeout(timeout, connect=10.0)) as client:
            with client.stream("POST", url, json=body) as resp:
                resp.raise_for_status()
                for line in resp.iter_lines():
                    if not line.startswith("data: "):
                        continue
                    payload = line[6:].strip()
                    if payload == "[DONE]":
                        break
                    try:
                        event = json.loads(payload)
                    except json.JSONDecodeError:
                        continue
                    usage = event.get("usage")
                    if usage and usage.get("completion_tokens"):
                        completion_tokens = usage["completion_tokens"]
                    choices = event.get("choices") or []
                    if not choices:
                        continue
                    delta = choices[0].get("delta") or {}
                    # Reasoning models (gemma-4 here) stream into `reasoning`, not
                    # `content`; reading only content captured nothing and graded the
                    # empty string as no_code, which is how gemma scored 2/8.
                    chunk = delta.get("content") or delta.get("reasoning") or ""
                    if chunk:
                        if ttft is None:
                            ttft = time.perf_counter() - start
                        parts.append(chunk)
    except Exception as exc:  # noqa: BLE001
        return {"error": str(exc)[:160], "elapsed_s": time.perf_counter() - start,
                "ttft_s": ttft, "tokens": completion_tokens or 0, "text": ""}
    elapsed = time.perf_counter() - start
    return {"error": None, "elapsed_s": elapsed, "ttft_s": ttft,
            "tokens": completion_tokens or 0, "text": "".join(parts)}


def run_level(target: str, level: int, tasks: list[dict[str, str]],
              timeout: float) -> dict[str, Any]:
    cfg = TARGETS[target]
    # gemma reasons before answering, so the 1024 cap truncates it mid-code; it needs
    # the harmony budget even though it isn't a harmony-format model
    reasoning = target in eval_code.THINKING_TARGETS or target == "gemma"
    max_tok = (eval_code.MAX_TOKENS_HARMONY
               if target in eval_code.HARMONY_TARGETS or reasoning
               else eval_code.MAX_TOKENS)

    sampler = RssSampler(lambda: _openai_pids(cfg["port"]))
    sampler.start()
    # max_workers caps how many requests are in flight at once; the pool keeps
    # exactly `level` running until the fixed workload is drained.
    results: list[dict[str, Any]] = []
    wall_start = time.perf_counter()
    with ThreadPoolExecutor(max_workers=level) as pool:
        futures = []
        for task in tasks:
            futures.append(pool.submit(
                one_request, port=cfg["port"], model=cfg["model"],
                prompt=prompt_for(task), max_tokens=max_tok, timeout=timeout))
        for f in futures:
            results.append(f.result())
    wall = time.perf_counter() - wall_start
    peak_rss = sampler.stop()

    # grade every answer: accuracy must not move with concurrency
    passed = 0
    statuses = []
    for task, r in zip(tasks, results):
        if r["error"]:
            statuses.append("http_error")
            continue
        text = r["text"]
        if target in eval_code.HARMONY_TARGETS:
            text = eval_code.strip_harmony(text)
        if target in eval_code.THINKING_TARGETS and "</think>" in text:
            text = text.split("</think>", 1)[1]
        code = eval_code.extract_code(text, task["sig"])
        with tempfile.TemporaryDirectory() as td:
            status, _ = eval_code.grade(task, code, Path(td))
        statuses.append(status)
        passed += status == "pass"

    ok = [r for r in results if not r["error"] and r["tokens"]]
    total_tokens = sum(r["tokens"] for r in ok)
    ttfts = [r["ttft_s"] for r in ok if r["ttft_s"] is not None]
    # per-stream decode rate, excluding the prefill each request paid for
    per_stream = [
        (r["tokens"] - 1) / (r["elapsed_s"] - r["ttft_s"])
        for r in ok if r["ttft_s"] is not None and r["elapsed_s"] > r["ttft_s"] and r["tokens"] > 1
    ]
    return {
        "level": level,
        "requests": len(results),
        "wall_s": round(wall, 2),
        "total_tokens": total_tokens,
        "aggregate_tok_s": round(total_tokens / wall, 2) if wall else None,
        "per_stream_tok_s": round(statistics.median(per_stream), 2) if per_stream else None,
        "ttft_ms_median": round(statistics.median(ttfts) * 1000, 1) if ttfts else None,
        "ttft_ms_max": round(max(ttfts) * 1000, 1) if ttfts else None,
        "peak_rss_mb": round(peak_rss, 1) if peak_rss else None,
        "passed": passed,
        "total": len(results),
        "statuses": statuses,
        "errors": [r["error"] for r in results if r["error"]][:3],
    }


def main() -> int:
    ap = argparse.ArgumentParser(description="Concurrent serving benchmark")
    ap.add_argument("--target", choices=tuple(TARGETS), required=True)
    ap.add_argument("--timeout", type=float, default=900.0)
    ap.add_argument("--levels", default=",".join(str(x) for x in LEVELS))
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    levels = [int(x) for x in args.levels.split(",") if x.strip()]
    tasks = build_prompts()
    started = dt.datetime.now()

    out_levels = []
    for level in levels:
        row = run_level(args.target, level, tasks, args.timeout)
        out_levels.append(row)
        print(f"  {args.target} concurrency {level}: {row['wall_s']}s wall, "
              f"{row['aggregate_tok_s']} tok/s aggregate, "
              f"{row['passed']}/{row['total']} correct", file=sys.stderr)
        time.sleep(3)

    base = next((l for l in out_levels if l["level"] == 1), None)
    for l in out_levels:
        if base and base["aggregate_tok_s"] and l["aggregate_tok_s"]:
            l["speedup_vs_1"] = round(l["aggregate_tok_s"] / base["aggregate_tok_s"], 2)
        else:
            l["speedup_vs_1"] = None

    doc = {
        "target": args.target,
        "model": TARGETS[args.target]["model"],
        "date": started.strftime("%Y-%m-%d %H:%M"),
        "workload": f"{WORKLOAD} C tasks, identical at every level",
        "tasks": [t["name"] for t in tasks],
        "levels": out_levels,
    }
    RESULTS.mkdir(parents=True, exist_ok=True)
    stamp = started.strftime("%Y%m%d-%H%M%S")
    path = RESULTS / f"{args.target}-{stamp}.json"
    path.write_text(json.dumps(doc, indent=2))
    print(f"wrote {path}", file=sys.stderr)
    if args.json:
        print(json.dumps(doc, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
