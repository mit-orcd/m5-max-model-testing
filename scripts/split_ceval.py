#!/usr/bin/env python3
"""Split a --set all C-eval JSON into easy (ceval) + hard (chard).

The first Linux sweep ran eval_code.py with the default --set all, so
results/<t>-ceval.json is 16 easy + 3 hard tasks (57 samples). The Mac
report treats those as two columns. This rewrites ceval to easy-only and
writes chard.json from the hard tasks already in the file. Original saved
as <t>-ceval-all.json.
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from eval_code import task_set  # noqa: E402

RESULTS = Path(__file__).resolve().parent.parent / "results"


def _load(path: Path):
    txt = path.read_text()
    m = re.search(r"^[\[{]$", txt, re.M)
    if not m:
        raise SystemExit(f"{path}: no JSON")
    return json.loads(txt[m.start():])


def _subset(row: dict, names: set[str], trials: int) -> dict:
    results = {k: v for k, v in row.get("results", {}).items() if k in names}
    notes = {k: v for k, v in row.get("notes", {}).items() if k in names}
    times = {k: v for k, v in row.get("time_s", {}).items() if k in names}
    tokens = {k: v for k, v in row.get("tokens", {}).items() if k in names}
    passed = sum(sum(1 for s in o if s == "pass") for o in results.values())
    total = len(results) * trials
    out = dict(row)
    out.update({
        "results": results,
        "notes": notes,
        "time_s": times,
        "tokens": tokens,
        "passed": passed,
        "total": total,
        "total_time_s": round(sum(sum(v) for v in times.values()), 1) if times else row.get("total_time_s"),
        "total_tokens": sum(sum(v) for v in tokens.values()) if tokens else row.get("total_tokens"),
    })
    return out


def split_one(path: Path) -> str:
    easy = {t["name"] for t in task_set("easy")}
    hard = {t["name"] for t in task_set("hard")}
    doc = _load(path)
    row = doc[0] if isinstance(doc, list) else doc
    names = set(row.get("results", {}))
    if names <= easy:
        return f"skip {path.name} (already easy-only)"
    if not (names & hard):
        return f"skip {path.name} (no hard tasks)"
    trials = int(row.get("trials") or 3)
    backup = path.with_name(path.name.replace("-ceval.json", "-ceval-all.json"))
    if not backup.exists():
        backup.write_text(path.read_text())
    easy_row = _subset(row, names & easy, trials)
    hard_row = _subset(row, names & hard, trials)
    path.write_text(json.dumps([easy_row], indent=2) + "\n")
    chard = path.with_name(path.name.replace("-ceval.json", "-chard.json"))
    if not chard.exists():
        chard.write_text(json.dumps([hard_row], indent=2) + "\n")
    return (
        f"{path.stem}: easy {easy_row['passed']}/{easy_row['total']} "
        f"hard {hard_row['passed']}/{hard_row['total']}"
    )


def main() -> None:
    paths = sorted(RESULTS.glob("*-ceval.json"))
    if len(sys.argv) > 1:
        paths = [Path(p) for p in sys.argv[1:]]
    for p in paths:
        print(split_one(p))


if __name__ == "__main__":
    main()
