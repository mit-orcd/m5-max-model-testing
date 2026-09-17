#!/usr/bin/env python3
"""Time the on-disk kimi-k3 referee solutions. No model call."""
from __future__ import annotations

import argparse
import datetime as dt
import json
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import eval_perf  # noqa: E402


FILES = {
    "range_sums": "range_sums.c",
    "dedupe": "py/dedupe.py",
    "top_freq": "sh/top_freq.sh",
}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--ref", type=Path, required=True,
                    help="results/referee/kimi-k3 directory")
    ap.add_argument("--out", type=Path, required=True,
                    help="perf results directory")
    args = ap.parse_args()
    variants: dict = {}
    for variant in ("silent", "told"):
        vres = {}
        for task in eval_perf.TASKS:
            code = (args.ref / FILES[task["name"]]).read_text()
            with tempfile.TemporaryDirectory() as td:
                status, ms, note = eval_perf.measure(task, code, Path(td))
            mss = [ms] if ms is not None else []
            vres[task["name"]] = {
                "statuses": [status],
                "ms": mss,
                "best_ms": ms,
                "median_ms": ms,
                "correct": 1 if status == "pass" else 0,
                "trials": 1,
                "note": note,
            }
            print(f"  {variant} {task['name']:11} {status} {ms if ms is not None else '—'}",
                  file=sys.stderr)
        variants[variant] = vres
    started = dt.datetime.now()
    doc = {
        "target": "kimi-k3",
        "model": "kimi-k3 (referee, cloud)",
        "note": "Referee solutions timed on this machine by eval_perf.measure; "
                "one run per variant, no generation.",
        "variants": variants,
        "date": started.strftime("%Y-%m-%d %H:%M"),
    }
    args.out.mkdir(parents=True, exist_ok=True)
    path = args.out / f"kimi-k3-{started.strftime('%Y%m%d-%H%M%S')}.json"
    path.write_text(json.dumps(doc, indent=2))
    print(f"wrote {path}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
