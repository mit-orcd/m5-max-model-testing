#!/usr/bin/env python3
"""Compare MLX :8080 vs Ollama :11434 on the local 27B Qwen pair.

Warmup is discarded. Median of --trials runs.
Ollama uses the native /api/chat eval_* timings (not OpenAI chunk math).
Do not run both servers at once if you want a clean GPU comparison.
"""

from __future__ import annotations

import argparse
import json
import statistics
import subprocess
import threading
import time
from collections.abc import Callable
from typing import Any

import httpx

TARGETS = {
    "mlx": {
        "base": "http://127.0.0.1:8080/v1",
        "model": "mlx-community/Qwen3.8-27B-4bit",
        "port": 8080,
        "other": 11434,
        "kind": "openai",
    },
    "ollama": {
        "base": "http://127.0.0.1:11434",
        "model": "qwen3.8:27b-mlx",
        "port": 11434,
        "other": 8080,
        "kind": "ollama",
    },
    "ornith": {
        "base": "http://127.0.0.1:8082/v1",
        "model": "mlx-community/Ornith-1.5-35B-A3B-OptiQ-4bit",
        "port": 8082,
        "other": 8080,
        "kind": "openai",
    },
    # Comparison set — served one at a time on :8083 (RAM budget).
    "coder": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3-Coder-30B-A3B-Instruct-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "qwen35": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3.5-35B-A3B-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "gptoss": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/gpt-oss-20b-MXFP4-Q8",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "gemma": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/gemma-4-26b-a4b-it-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "devstral": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Devstral-Small-2-24B-Instruct-2512-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "qwen27": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3.8-27B-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "aya": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/aya-23-35B-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "qwen36-27b": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3.6-27B-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "qwen36-35b": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3.6-35B-A3B-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "glm-flash": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/GLM-4.7-Flash-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "coder-next": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3-Coder-Next-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "deepseek-32b": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/DeepSeek-R1-Distill-Qwen-32B-MLX-4Bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
}

DECODE_PROMPT = (
    "Write a 1500-word paragraph about MIT and its influence on AI research and discoveries. Do not use a title."
)
PREFILL_UNIT = (
    "MIT has shaped AI research through pioneering labs, influential alumni, and decades of discovery. "
)
CASES = {
    "decode": {"max_tokens": 2048, "prompt": DECODE_PROMPT},
    "prefill": {
        "max_tokens": 128,
        "prompt": (PREFILL_UNIT * 670)
        + "\n\nSummarize the above in exactly three short bullet points.",
    },
}
MIN_DECODE_S = 0.05

# Deterministic quality probes. Each checker gets the raw reply text.
QUALITY_TASKS: list[tuple[str, Any]] = [
    ("arith", lambda t: "408" in t.replace(",", "")),
    ("exact", lambda t: t.strip() == "OK"),
    ("json", lambda t: _is_json_object(t)),
    ("primes", lambda t: all(p in t for p in ("2", "3", "5", "7", "11"))),
    ("code", lambda t: "[::-1]" in t.replace(" ", "")),
    ("follow", lambda t: t.strip().lower().startswith("red")),
]
QUALITY_PROMPTS = {
    "arith": "What is 17 * 24? Reply with only the number.",
    "exact": "Reply with exactly this token and nothing else: OK",
    "json": 'Output only a JSON object {"a": 1}. No prose, no code fences.',
    "primes": "List the first five prime numbers, comma separated, nothing else.",
    "code": "Write a Python expression (no prose) that reverses a string s.",
    "follow": "Answer in one lowercase word: what color is a ripe strawberry?",
}
QUALITY_MAX_TOKENS = 64


def _is_json_object(text: str) -> bool:
    try:
        return isinstance(json.loads(text.strip()), dict)
    except (json.JSONDecodeError, ValueError):
        return False


def _median(xs: list[float]) -> float | None:
    return statistics.median(xs) if xs else None


def _port_up(port: int) -> bool:
    try:
        r = httpx.get(f"http://127.0.0.1:{port}/v1/models", timeout=2.0)
        return r.status_code < 500
    except (httpx.RequestError, httpx.HTTPStatusError):
        return False


def _listen_pid(port: int) -> int | None:
    try:
        out = subprocess.check_output(
            ["lsof", "-tiTCP:%d" % port, "-sTCP:LISTEN"],
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
    except subprocess.CalledProcessError:
        return None
    if not out:
        return None
    return int(out.split()[0])


def _pgrep(args: list[str]) -> list[int]:
    try:
        out = subprocess.check_output(
            ["pgrep", *args], text=True, stderr=subprocess.DEVNULL
        )
    except subprocess.CalledProcessError:
        return []
    pids = []
    for line in out.split():
        try:
            pids.append(int(line))
        except ValueError:
            continue
    return pids


def _mlx_pids() -> list[int]:
    pid = _listen_pid(8080)
    return [pid] if pid else []


def _openai_pids(port: int) -> list[int]:
    pid = _listen_pid(port)
    return [pid] if pid else []


def _ollama_pids() -> list[int]:
    pids = set(_pgrep(["-x", "ollama"]))
    pids.update(_pgrep(["-f", "ollama runner"]))
    listen = _listen_pid(11434)
    if listen:
        pids.add(listen)
        pids.update(_pgrep(["-P", str(listen)]))
    return [p for p in pids if p]


def _rss_mb(pid: int) -> float | None:
    try:
        out = subprocess.check_output(
            ["ps", "-o", "rss=", "-p", str(pid)],
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
    except subprocess.CalledProcessError:
        return None
    if not out:
        return None
    return int(out.split()[0]) / 1024.0


class RssSampler:
    def __init__(self, pid_fn: Callable[[], list[int]], interval: float = 0.05):
        self.pid_fn = pid_fn
        self.interval = interval
        self.peak: float | None = None
        self._stop = threading.Event()
        self._thread: threading.Thread | None = None

    def start(self) -> None:
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def _sum_rss(self) -> float | None:
        total = 0.0
        found = False
        for pid in self.pid_fn():
            mb = _rss_mb(pid)
            if mb is None:
                continue
            total += mb
            found = True
        return total if found else None

    def _run(self) -> None:
        while not self._stop.is_set():
            mb = self._sum_rss()
            if mb is not None:
                self.peak = mb if self.peak is None else max(self.peak, mb)
            self._stop.wait(self.interval)

    def stop(self) -> float | None:
        self._stop.set()
        if self._thread is not None:
            self._thread.join(timeout=1.0)
        return self.peak


def _first_str(*values: Any) -> str:
    for value in values:
        if isinstance(value, str) and value:
            return value
    return ""


def _openai_visible(event: dict[str, Any]) -> str:
    choices = event.get("choices") or []
    if not choices:
        return ""
    choice = choices[0]
    delta = choice.get("delta") or {}
    message = choice.get("message") or {}
    return _first_str(
        delta.get("content"),
        delta.get("reasoning_content"),
        delta.get("reasoning"),
        message.get("content"),
        message.get("reasoning_content"),
        message.get("reasoning"),
    )


def _ollama_visible(event: dict[str, Any]) -> str:
    message = event.get("message") or {}
    return _first_str(
        message.get("content"),
        message.get("thinking"),
        message.get("reasoning"),
        event.get("response"),
    )


def _sse_payloads(line: str) -> dict[str, Any] | None:
    line = line.strip()
    if not line.startswith("data:"):
        return None
    data = line[5:].strip()
    if not data or data == "[DONE]":
        return None
    try:
        obj = json.loads(data)
    except json.JSONDecodeError:
        return None
    return obj if isinstance(obj, dict) else None


def _ndjson(line: str) -> dict[str, Any] | None:
    line = line.strip()
    if not line:
        return None
    try:
        obj = json.loads(line)
    except json.JSONDecodeError:
        return None
    return obj if isinstance(obj, dict) else None


def _tok_s_from_ns(count: int | None, duration_ns: int | None) -> float | None:
    if not isinstance(count, int) or count < 1:
        return None
    if not isinstance(duration_ns, int) or duration_ns <= 0:
        return None
    return count / (duration_ns / 1e9)


def _tok_s_wall(completion_tokens: int, elapsed: float, ttft: float | None) -> float | None:
    if completion_tokens < 1 or elapsed <= 0:
        return None
    if ttft is None:
        return completion_tokens / elapsed
    decode_s = elapsed - ttft
    if decode_s < MIN_DECODE_S:
        return None
    return completion_tokens / decode_s


def stream_mlx(
    *,
    model: str,
    prompt: str,
    max_tokens: int,
    timeout: float,
    port: int = 8080,
    thinking: bool = False,
) -> dict[str, Any]:
    body: dict[str, Any] = {
        "model": model,
        "messages": [{"role": "user", "content": prompt}],
        "max_tokens": max_tokens,
        "temperature": 0,
        "stream": True,
        "stream_options": {"include_usage": True},
        "chat_template_kwargs": {"enable_thinking": thinking},
    }
    sampler = RssSampler(lambda: _openai_pids(port))
    url = f"http://127.0.0.1:{port}/v1/chat/completions"
    start = time.perf_counter()
    ttft: float | None = None
    text_parts: list[str] = []
    completion_tokens: int | None = None
    prompt_tokens: int | None = None
    tok_s: float | None = None
    metric: str | None = None
    sampler.start()
    try:
        with httpx.Client(timeout=httpx.Timeout(timeout, connect=5.0)) as client:
            with client.stream("POST", url, json=body) as resp:
                resp.raise_for_status()
                for line in resp.iter_lines():
                    event = _sse_payloads(line)
                    if event is None:
                        continue
                    usage = event.get("usage")
                    if isinstance(usage, dict):
                        if isinstance(usage.get("completion_tokens"), int):
                            completion_tokens = usage["completion_tokens"]
                        if isinstance(usage.get("prompt_tokens"), int):
                            prompt_tokens = usage["prompt_tokens"]
                    timings = event.get("timings")
                    if isinstance(timings, dict):
                        predicted = timings.get("predicted_per_second")
                        if isinstance(predicted, (int, float)) and predicted > 0:
                            tok_s = float(predicted)
                            metric = "mlx_timings"
                    piece = _openai_visible(event)
                    if not piece:
                        continue
                    if ttft is None:
                        ttft = time.perf_counter() - start
                    text_parts.append(piece)
    finally:
        peak = sampler.stop()
    elapsed = time.perf_counter() - start
    text = "".join(text_parts)
    if completion_tokens is None:
        completion_tokens = max(1, (len(text) + 3) // 4) if text else 0
    if tok_s is None:
        tok_s = _tok_s_wall(completion_tokens, elapsed, ttft)
        if tok_s is not None:
            metric = "wall"
    return {
        "ttft_ms": None if ttft is None else ttft * 1000.0,
        "elapsed_s": elapsed,
        "tok_s": tok_s,
        "completion_tokens": completion_tokens,
        "prompt_tokens": prompt_tokens,
        "peak_rss_mb": peak,
        "chars": len(text),
        "metric": metric,
    }


def stream_ollama(
    *,
    model: str,
    prompt: str,
    max_tokens: int,
    timeout: float,
) -> dict[str, Any]:
    body: dict[str, Any] = {
        "model": model,
        "messages": [{"role": "user", "content": prompt}],
        "stream": True,
        "think": False,
        "keep_alive": "10m",
        "options": {
            "temperature": 0,
            "num_predict": max_tokens,
            "num_ctx": 32768,
        },
    }
    sampler = RssSampler(_ollama_pids)
    url = TARGETS["ollama"]["base"].rstrip("/") + "/api/chat"
    start = time.perf_counter()
    ttft: float | None = None
    text_parts: list[str] = []
    completion_tokens: int | None = None
    prompt_tokens: int | None = None
    tok_s: float | None = None
    metric: str | None = None
    sampler.start()
    try:
        with httpx.Client(timeout=httpx.Timeout(timeout, connect=5.0)) as client:
            with client.stream("POST", url, json=body) as resp:
                resp.raise_for_status()
                for line in resp.iter_lines():
                    event = _ndjson(line)
                    if event is None:
                        continue
                    if event.get("done"):
                        prompt_tokens = event.get("prompt_eval_count", prompt_tokens)
                        completion_tokens = event.get("eval_count", completion_tokens)
                        native = _tok_s_from_ns(
                            event.get("eval_count"), event.get("eval_duration")
                        )
                        if native is not None:
                            tok_s = native
                            metric = "ollama_eval"
                    piece = _ollama_visible(event)
                    if not piece:
                        continue
                    if ttft is None:
                        ttft = time.perf_counter() - start
                    text_parts.append(piece)
    finally:
        peak = sampler.stop()
    elapsed = time.perf_counter() - start
    text = "".join(text_parts)
    if completion_tokens is None:
        completion_tokens = max(1, (len(text) + 3) // 4) if text else 0
    if tok_s is None:
        tok_s = _tok_s_wall(completion_tokens, elapsed, ttft)
        if tok_s is not None:
            metric = "wall"
    return {
        "ttft_ms": None if ttft is None else ttft * 1000.0,
        "elapsed_s": elapsed,
        "tok_s": tok_s,
        "completion_tokens": completion_tokens,
        "prompt_tokens": prompt_tokens,
        "peak_rss_mb": peak,
        "chars": len(text),
        "metric": metric,
    }


def complete_openai(
    *, port: int, model: str, prompt: str, max_tokens: int, timeout: float,
    temperature: float = 0.0,
) -> str:
    return complete_openai_full(
        port=port, model=model, prompt=prompt, max_tokens=max_tokens,
        timeout=timeout, temperature=temperature,
    )["text"]


def complete_openai_full(
    *, port: int, model: str, prompt: str, max_tokens: int, timeout: float,
    temperature: float = 0.0,
) -> dict[str, Any]:
    """Like complete_openai but also returns wall time and token usage."""
    body = {
        "model": model,
        "messages": [{"role": "user", "content": prompt}],
        "max_tokens": max_tokens,
        "temperature": temperature,
        "stream": False,
        "chat_template_kwargs": {"enable_thinking": False},
    }
    url = f"http://127.0.0.1:{port}/v1/chat/completions"
    start = time.perf_counter()
    with httpx.Client(timeout=httpx.Timeout(timeout, connect=5.0)) as client:
        resp = client.post(url, json=body)
        resp.raise_for_status()
        data = resp.json()
    elapsed = time.perf_counter() - start
    choices = data.get("choices") or []
    message = choices[0].get("message") if choices else {}
    content = (message or {}).get("content")
    usage = data.get("usage") or {}
    return {
        "text": content if isinstance(content, str) else "",
        "elapsed_s": elapsed,
        "completion_tokens": usage.get("completion_tokens"),
        "prompt_tokens": usage.get("prompt_tokens"),
    }


def complete_ollama(
    *, model: str, prompt: str, max_tokens: int, timeout: float
) -> str:
    body = {
        "model": model,
        "messages": [{"role": "user", "content": prompt}],
        "stream": False,
        "think": False,
        "options": {"temperature": 0, "num_predict": max_tokens},
    }
    url = TARGETS["ollama"]["base"].rstrip("/") + "/api/chat"
    with httpx.Client(timeout=httpx.Timeout(timeout, connect=5.0)) as client:
        resp = client.post(url, json=body)
        resp.raise_for_status()
        data = resp.json()
    message = data.get("message") or {}
    content = message.get("content")
    return content if isinstance(content, str) else ""


def run_quality(name: str, timeout: float) -> dict[str, Any]:
    cfg = TARGETS[name]
    if not _port_up(cfg["port"]):
        raise SystemExit(f"{name} is not up on :{cfg['port']}")
    results: dict[str, bool] = {}
    replies: dict[str, str] = {}
    for task, checker in QUALITY_TASKS:
        prompt = QUALITY_PROMPTS[task]
        if cfg["kind"] == "ollama":
            reply = complete_ollama(
                model=cfg["model"],
                prompt=prompt,
                max_tokens=QUALITY_MAX_TOKENS,
                timeout=timeout,
            )
        else:
            reply = complete_openai(
                port=cfg["port"],
                model=cfg["model"],
                prompt=prompt,
                max_tokens=QUALITY_MAX_TOKENS,
                timeout=timeout,
            )
        replies[task] = reply.strip()[:120]
        results[task] = bool(checker(reply))
        print(f"{name} quality {task}: {'pass' if results[task] else 'FAIL'}", flush=True)
    passed = sum(results.values())
    return {
        "target": name,
        "model": cfg["model"],
        "case": "quality",
        "passed": passed,
        "total": len(QUALITY_TASKS),
        "score": passed / len(QUALITY_TASKS),
        "results": results,
        "replies": replies,
    }


def run_once(
    name: str, prompt: str, max_tokens: int, timeout: float, thinking: bool
) -> dict[str, Any]:
    cfg = TARGETS[name]
    if cfg["kind"] == "ollama":
        return stream_ollama(
            model=cfg["model"],
            prompt=prompt,
            max_tokens=max_tokens,
            timeout=timeout,
        )
    return stream_mlx(
        model=cfg["model"],
        prompt=prompt,
        max_tokens=max_tokens,
        timeout=timeout,
        port=cfg["port"],
        thinking=thinking,
    )


def run_target(
    name: str, cases: list[str], trials: int, timeout: float, thinking: bool
) -> list[dict[str, Any]]:
    cfg = TARGETS[name]
    if not _port_up(cfg["port"]):
        raise SystemExit(f"{name} is not up on :{cfg['port']}")
    if _port_up(cfg["other"]):
        print(
            f"warning: the other server is also listening on :{cfg['other']}; "
            "GPU results will be contaminated. Stop it for a fair run.",
            flush=True,
        )
    print(f"warmup {name}...", flush=True)
    run_once(name, "Reply with the single word ping.", 8, timeout, thinking)
    rows: list[dict[str, Any]] = []
    for case in cases:
        spec = CASES[case]
        runs = []
        for i in range(trials):
            print(f"{name} {case} {i + 1}/{trials}...", flush=True)
            runs.append(
                run_once(name, spec["prompt"], spec["max_tokens"], timeout, thinking)
            )
        ttfts = [r["ttft_ms"] for r in runs if r["ttft_ms"] is not None]
        toks = [r["tok_s"] for r in runs if r["tok_s"] is not None]
        rss = [r["peak_rss_mb"] for r in runs if r["peak_rss_mb"] is not None]
        outs = [r["completion_tokens"] for r in runs if r["completion_tokens"]]
        prompt_n = next((r["prompt_tokens"] for r in runs if r["prompt_tokens"]), None)
        metric = next((r["metric"] for r in runs if r.get("metric")), None)
        rows.append(
            {
                "target": name,
                "model": cfg["model"],
                "case": case,
                "trials": trials,
                "prompt_tokens": prompt_n,
                "out_tok": int(_median([float(x) for x in outs])) if outs else None,
                "ttft_ms": _median(ttfts),
                "tok_s": _median(toks),
                "peak_rss_mb": _median(rss),
                "metric": metric,
                "thinking": thinking,
                "runs": runs,
            }
        )
    return rows


def _fmt(v: float | None, digits: int = 1) -> str:
    return "—" if v is None else f"{v:.{digits}f}"


def print_table(rows: list[dict[str, Any]]) -> None:
    print()
    print(
        f"{'target':<8} {'case':<8} {'think':>5} {'ttft_ms':>10} {'tok/s':>8} "
        f"{'peak_rss_mb':>12} {'prompt_tok':>11} {'out_tok':>8} {'metric':<12}"
    )
    for row in rows:
        print(
            f"{row['target']:<8} {row['case']:<8} "
            f"{str(row.get('thinking')):>5} "
            f"{_fmt(row['ttft_ms']):>10} {_fmt(row['tok_s']):>8} "
            f"{_fmt(row['peak_rss_mb']):>12} "
            f"{'—' if row['prompt_tokens'] is None else str(row['prompt_tokens']):>11} "
            f"{'—' if row['out_tok'] is None else str(row['out_tok']):>8} "
            f"{row.get('metric') or '—':<12}"
        )


def print_quality(rows: list[dict[str, Any]]) -> None:
    print()
    print(f"{'target':<8} {'score':>10}  details")
    for row in rows:
        marks = " ".join(
            f"{k}:{'✓' if v else '✗'}" for k, v in row["results"].items()
        )
        print(f"{row['target']:<8} {row['passed']}/{row['total']:<8}  {marks}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Bench MLX vs Ollama vs Ornith (local)")
    parser.add_argument("--target", choices=tuple(TARGETS) + ("both",), default="both")
    parser.add_argument("--case", choices=("decode", "prefill", "quality", "both"), default="both")
    parser.add_argument("--trials", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=180.0)
    parser.add_argument("--thinking", action="store_true", help="enable thinking on OpenAI targets")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    if args.case == "quality":
        names = ["mlx", "ollama"] if args.target == "both" else [args.target]
        rows = [run_quality(name, args.timeout) for name in names]
        if args.json:
            print(json.dumps(rows, indent=2))
        else:
            print_quality(rows)
        return
    cases = ["decode", "prefill"] if args.case == "both" else [args.case]
    names = ["mlx", "ollama"] if args.target == "both" else [args.target]
    rows: list[dict[str, Any]] = []
    for name in names:
        rows.extend(run_target(name, cases, args.trials, args.timeout, args.thinking))
    if args.json:
        print(json.dumps(rows, indent=2))
    else:
        print_table(rows)


if __name__ == "__main__":
    main()
