#!/usr/bin/env python3
"""Print per-sample and per-model results for the c-arena build."""
import subprocess
from collections import defaultdict
from pathlib import Path

TASKS = ['atoi_strict', 'binary_search', 'bswap32', 'cmp_desc', 'count_words', 'fizzbuzz', 'is_prime', 'itoa', 'max_subarray', 'parse_csv_ints', 'popcount', 'reverse_string', 'ring', 'rot13', 'stack', 'trim']

def parse(stem):
    if stem.startswith("referee-"):
        return "referee", stem.removeprefix("referee-"), "-"
    base, _, trial = stem.rpartition("-t")
    for t in TASKS:
        if base.endswith("-" + t):
            return base[: -len(t) - 1], t, trial
    return base, "?", trial

SAMPLES = sorted(Path("samples").glob("*.c"))
rows = []
agg = defaultdict(lambda: [0, 0, 0])  # model -> [pass, test_fail, compile_fail]
for s in SAMPLES:
    stem = s.stem
    b = Path("build") / stem
    if Path(str(b) + ".COMPILE_FAIL").exists():
        result = "COMPILE FAIL"
    elif b.exists():
        r = subprocess.run([str(b)], capture_output=True, timeout=10)
        result = "PASS" if r.returncode == 0 else "TEST FAIL"
    else:
        result = "COMPILE FAIL"
    model, task, trial = parse(stem)
    rows.append((model, task, trial, result))
    agg[model][{"PASS": 0, "TEST FAIL": 1, "COMPILE FAIL": 2}[result]] += 1

print(f"{'model':<16} {'task':<16} {'t':<3} result")
print("-" * 50)
for model, task, trial, result in rows:
    print(f"{model:<16} {task:<16} {trial:<3} {result}")
print()
print(f"{'model':<16} {'pass':>5} {'test-fail':>9} {'compile-fail':>12}")
print("-" * 46)
for model in sorted(agg):
    p, tf, cf = agg[model]
    print(f"{model:<16} {p:>5} {tf:>9} {cf:>12}")
