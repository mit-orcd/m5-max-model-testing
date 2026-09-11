#!/usr/bin/env python3
"""Does how you frame the request change the code you get?

We already know one framing matters enormously: adding "its running time will be
measured" to the C range-sums prompt makes three of five models switch from an
O(n*q) loop to a prefix sum, 300-470x faster. This asks how far that generalises.
Is it that the model was told *what will be judged*, or would any authoritative
framing -- claimed expertise, a persona, raised stakes -- do the same?

The published evidence on personas is not encouraging ("When 'A Helpful Assistant'
Is Not Really Helpful", 2024, found they do not reliably improve accuracy), so the
hypothesis under test is that task-relevant information works and identity framing
does not.

The probe is deliberately binary: the model either writes the prefix sum or it does
not, and the two are ~400x apart in runtime, so classification is unambiguous. One
greedy sample at temperature 0 plus N samples at 0.7 per condition, reported with
Wilson confidence intervals and Fisher exact tests against the bare prompt --
because a 20%-to-40% shift is invisible at the 3 trials the other evals use.
"""
from __future__ import annotations

import argparse
import datetime as dt
import json
import math
import sys
import tempfile
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))

import eval_code
import eval_perf
from bench import TARGETS, complete_openai_full

RESULTS = Path(__file__).resolve().parent.parent / "results" / "framing"
FAST_MS = 10.0   # reference fast answer is 0.29 ms, naive is ~123 ms

# The dependent variable here is the algorithm, not whether the model remembered
# its headers -- and those two are not independent: the prefix sum needs malloc
# and so needs <stdlib.h>, while the naive loop allocates nothing and never trips
# the wire. Left alone, a forgotten include turns a fast answer into a compile
# error and biases every framing that works *downward*. Header guards make the
# duplicates harmless. (eval_perf.py deliberately does not do this: it measures
# working code end to end, and its published numbers stay as they are.)
PREAMBLE = ("#include <stddef.h>\n#include <stdlib.h>\n#include <string.h>\n"
            "#include <stdint.h>\n#include <limits.h>\n")

# prefix goes before the instruction, suffix after the task description
CONDITIONS: list[dict[str, str]] = [
    {"name": "bare", "prefix": "", "suffix": "",
     "what": "no framing at all — the baseline"},
    {"name": "timed", "prefix": "",
     "suffix": " It will be called with n and q both in the hundreds of thousands, "
               "and its running time will be measured.",
     "what": "names the success criterion — positive control"},
    {"name": "user_expert", "prefix": "I am a very experienced C developer. ", "suffix": "",
     "what": "claims the asker is an expert"},
    {"name": "model_persona",
     "prefix": "You are a senior systems programmer with 20 years of C experience. ",
     "suffix": "", "what": "assigns the model an expert persona"},
    {"name": "stakes", "prefix": "",
     "suffix": " This is production code and it will be reviewed.",
     "what": "raises stakes without naming the criterion"},
    {"name": "think", "prefix": "",
     "suffix": " Think carefully about the algorithm before writing.",
     "what": "nudges toward deliberation, no domain hint"},
    {"name": "user_beginner", "prefix": "I am a beginner learning C. ", "suffix": "",
     "what": "negative control — does it get worse?"},
]

TASK = next(t for t in eval_perf.TASKS if t["name"] == "range_sums")


def build_prompt(cond: dict[str, str]) -> str:
    return (f"{cond['prefix']}Implement in C11: `{TASK['sig']}`. {TASK['task']}"
            f"{cond['suffix']}\n"
            "Reply with only a C code block. No main function, no tests, no explanation.")


# ------------------------------------------------------------------ statistics

def wilson(k: int, n: int, z: float = 1.96) -> tuple[float, float]:
    """95% Wilson score interval — behaves sensibly at 0 and at n, unlike normal."""
    if n == 0:
        return (0.0, 0.0)
    p = k / n
    d = 1 + z * z / n
    centre = (p + z * z / (2 * n)) / d
    half = z * math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / d
    return (max(0.0, centre - half), min(1.0, centre + half))


def fisher_exact(a: int, b: int, c: int, d: int) -> float:
    """Two-sided Fisher exact p-value for [[a,b],[c,d]]. Exact, no scipy needed."""
    n = a + b + c + d
    row1, col1 = a + b, a + c

    def prob(x: int) -> float:
        return (math.comb(row1, x) * math.comb(n - row1, col1 - x)) / math.comb(n, col1)

    lo = max(0, col1 - (n - row1))
    hi = min(row1, col1)
    observed = prob(a)
    total = 0.0
    for x in range(lo, hi + 1):
        p = prob(x)
        if p <= observed * (1 + 1e-9):
            total += p
    return min(1.0, total)


# --------------------------------------------------------------------- driver

def classify(target: str, prompt: str, temperature: float,
             timeout: float, max_tok: int) -> tuple[str, float | None]:
    cfg = TARGETS[target]
    try:
        resp = complete_openai_full(port=cfg["port"], model=cfg["model"], prompt=prompt,
                                    max_tokens=max_tok, timeout=timeout,
                                    temperature=temperature)
    except Exception:  # noqa: BLE001
        return "http_error", None
    reply = resp["text"]
    if target in eval_code.HARMONY_TARGETS:
        reply = eval_code.strip_harmony(reply)
    if target in eval_code.THINKING_TARGETS and "</think>" in reply:
        reply = reply.split("</think>", 1)[1]
    code = eval_code.extract_code(reply, TASK["sig"])
    with tempfile.TemporaryDirectory() as td:
        status, ms, _note = eval_perf.measure(TASK, PREAMBLE + code, Path(td))
    if status != "pass":
        return status, None
    return ("fast" if ms is not None and ms < FAST_MS else "slow"), ms


def run_target(target: str, trials: int, timeout: float) -> dict[str, Any]:
    max_tok = (eval_code.MAX_TOKENS_HARMONY
               if target in eval_code.HARMONY_TARGETS or target in eval_code.THINKING_TARGETS
               else eval_code.MAX_TOKENS)
    out: dict[str, Any] = {"target": target, "model": TARGETS[target]["model"],
                           "trials_sampled": trials, "conditions": {}}
    for cond in CONDITIONS:
        prompt = build_prompt(cond)
        greedy, _ = classify(target, prompt, 0.0, timeout, max_tok)
        outcomes, times = [], []
        for _ in range(trials):
            o, ms = classify(target, prompt, 0.7, timeout, max_tok)
            outcomes.append(o)
            if ms is not None:
                times.append(ms)
        fast = sum(1 for o in outcomes if o == "fast")
        correct = sum(1 for o in outcomes if o in ("fast", "slow"))
        lo, hi = wilson(fast, len(outcomes))
        out["conditions"][cond["name"]] = {
            "what": cond["what"],
            "prompt": prompt,
            "greedy": greedy,
            "outcomes": outcomes,
            "fast": fast,
            "correct": correct,
            "n": len(outcomes),
            "fast_rate": round(fast / len(outcomes), 3) if outcomes else None,
            "ci95": [round(lo, 3), round(hi, 3)],
            "best_ms": round(min(times), 3) if times else None,
        }
        print(f"  {target} {cond['name']:14} greedy={greedy:9} "
              f"fast {fast}/{len(outcomes)} "
              f"[{lo:.0%}-{hi:.0%}]  correct {correct}/{len(outcomes)}", file=sys.stderr)

    base = out["conditions"].get("bare")
    if base:
        for name, c in out["conditions"].items():
            if name == "bare":
                c["p_vs_bare"] = None
                continue
            c["p_vs_bare"] = round(fisher_exact(
                c["fast"], c["n"] - c["fast"],
                base["fast"], base["n"] - base["fast"]), 4)
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description="Prompt-framing sensitivity experiment")
    ap.add_argument("--target", choices=tuple(TARGETS), required=True)
    ap.add_argument("--trials", type=int, default=20, help="samples at temp 0.7 per condition")
    ap.add_argument("--timeout", type=float, default=600.0)
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    started = dt.datetime.now()
    doc = run_target(args.target, args.trials, args.timeout)
    doc["date"] = started.strftime("%Y-%m-%d %H:%M")
    RESULTS.mkdir(parents=True, exist_ok=True)
    path = RESULTS / f"{args.target}-{started.strftime('%Y%m%d-%H%M%S')}.json"
    path.write_text(json.dumps(doc, indent=2))
    print(f"wrote {path}", file=sys.stderr)
    if args.json:
        print(json.dumps(doc, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
