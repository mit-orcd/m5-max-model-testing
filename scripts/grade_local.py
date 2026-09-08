#!/usr/bin/env python3
"""Grade local .c solution files against the eval_code harness.

Usage:
  scripts/grade_local.py <dir>          # dir contains <task>.c files
  scripts/grade_local.py --failures     # re-grade results/failures (audit)

Exit code 0 always; prints per-task verdict and total.
"""
from __future__ import annotations

import re
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from eval_code import TASKS, grade  # noqa: E402

TASK_BY_NAME = {t["name"]: t for t in TASKS}


def grade_file(path: Path, task: dict) -> tuple[str, str]:
    code = path.read_text()
    with tempfile.TemporaryDirectory() as td:
        return grade(task, code, Path(td))


def main() -> None:
    if sys.argv[1] == "--failures":
        # Audit: every dumped failure should still fail under the harness.
        fdir = Path(__file__).parent.parent / "results" / "failures"
        files = sorted(fdir.glob("*.c"))
        still_fail = passes = no_task = 0
        surprises = []
        for f in files:
            # <model-with-dashes>-<task_with_underscores>-t<trial>.c
            m = re.search(r"-([a-z0-9_]+)-t\d+$", f.stem)
            task_name = m.group(1) if m else ""
            task = TASK_BY_NAME.get(task_name)
            if task is None:
                no_task += 1
                continue
            status, _ = grade_file(f, task)
            if status == "pass":
                passes += 1
                surprises.append(f.name)
            else:
                still_fail += 1
        print(f"audit: {still_fail} still fail, {passes} unexpectedly pass, {no_task} unmapped")
        for name in surprises:
            print(f"  FALSE FAILURE: {name}")
        return

    sol_dir = Path(sys.argv[1])
    passed = 0
    total = 0
    for lang, tasks, grader in _suites():
        for task in tasks:
            f = sol_dir / lang / f"{task['name']}.{ 'py' if lang == 'py' else 'sh'}"
            if not f.exists():
                # C solutions live directly in sol_dir
                f = sol_dir / f"{task['name']}.c" if lang == "c" else f
            if not f.exists():
                continue
            total += 1
            status, note = grader(f, task)
            passed += status == "pass"
            print(f"{lang}/{task['name']:<16} {status:<14} {note}")
    print(f"\ntotal: {passed}/{total}")


def _suites():
    import eval_python
    import eval_bash
    py = str(Path(__file__).parent.parent / ".venv" / "bin" / "python")
    return [
        ("c", TASKS, grade_file),
        ("py", eval_python.TASKS,
         lambda f, t: eval_python.grade(t, f.read_text(), Path(tempfile.mkdtemp()), py)),
        ("sh", eval_bash.TASKS,
         lambda f, t: eval_bash.grade(t, f.read_text(), Path(tempfile.mkdtemp()))),
    ]


if __name__ == "__main__":
    main()
