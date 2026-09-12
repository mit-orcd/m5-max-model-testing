#!/usr/bin/env python3
"""Aggregate the framing experiment across models and test each framing for a real effect.

Two models (the gpt-oss pair) already write the prefix sum unprompted, so they sit at
the ceiling and can only show a framing doing harm. The headroom models fail by default,
which is where any effect has room to appear -- so the headline pools those four and
tests each framing against the bare prompt with a Fisher exact test.

Pooling raises the sample from 20 to 80 per condition, which is what makes a modest
effect detectable at all; a 25-point shift is invisible at n=20.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from eval_framing import CONDITIONS, fisher_exact, wilson

RESULTS = Path(__file__).resolve().parent.parent / "results" / "framing"
# the models with room to move: they write the naive loop when asked plainly
HEADROOM = ["coder-next", "qwen38flash", "qwen27", "laguna21"]
CEILING = ["gptoss", "gptoss120"]
ORDER = [c["name"] for c in CONDITIONS]


def latest() -> dict[str, dict]:
    newest: dict[str, tuple[str, dict]] = {}
    for p in sorted(RESULTS.glob("*.json")):
        doc = json.loads(p.read_text())
        t = doc["target"]
        if t not in newest or p.name > newest[t][0]:
            newest[t] = (p.name, doc)
    return {t: d for t, (_, d) in newest.items()}


def main() -> int:
    docs = latest()
    if not docs:
        print("no framing results yet", file=sys.stderr)
        return 1

    targets = [t for t in HEADROOM + CEILING if t in docs]
    print("\nFast-algorithm rate per model (n=20 sampled at temp 0.7)\n")
    head = f"{'condition':15}" + "".join(f"{t:>14}" for t in targets)
    print(head); print("-" * len(head))
    for name in ORDER:
        row = f"{name:15}"
        for t in targets:
            c = docs[t]["conditions"].get(name)
            row += f"{c['fast']:>8}/{c['n']:<5}" if c else f"{'-':>14}"
        print(row)

    pooled = [t for t in HEADROOM if t in docs]
    if not pooled:
        return 0
    print(f"\n\nPooled across the {len(pooled)} models with headroom "
          f"({', '.join(pooled)})\n")
    print(f"{'condition':15} {'fast':>9} {'rate':>7} {'95% CI':>14} {'p vs bare':>11}   what it tests")
    print("-" * 104)

    agg = {}
    for name in ORDER:
        f = sum(docs[t]["conditions"][name]["fast"] for t in pooled)
        n = sum(docs[t]["conditions"][name]["n"] for t in pooled)
        agg[name] = (f, n)

    bf, bn = agg["bare"]
    for name in ORDER:
        f, n = agg[name]
        lo, hi = wilson(f, n)
        p = None if name == "bare" else fisher_exact(f, n - f, bf, bn - bf)
        pstr = "—" if p is None else (f"{p:.2g}" + ("*" if p < 0.05 else ""))
        what = next(c["what"] for c in CONDITIONS if c["name"] == name)
        print(f"{name:15} {f:>4}/{n:<4} {f/n:>6.0%} "
              f"{f'{lo:.0%}-{hi:.0%}':>14} {pstr:>11}   {what}")
    print("\n* p < 0.05 (Fisher exact, two-sided). Greedy samples at temp 0 are excluded\n"
          "  from these counts and reported separately in the JSON.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
