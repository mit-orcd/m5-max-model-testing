#!/usr/bin/env python3
"""Checks the performance tasks before any model sees them.

For each task, two solutions go through the real harness: the obvious naive one
and the good one. Both must be *correct* — that is the whole design, since the
tasks are meant to be easy to get right — and the good one must be dramatically
faster. If the gap is small the task measures nothing.
"""
from __future__ import annotations

import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import eval_perf

C_NAIVE = r"""
#include <stddef.h>
void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi,
                size_t q, long long *out) {
    (void)n;
    for (size_t i = 0; i < q; i++) {
        long long s = 0;
        for (size_t j = lo[i]; j <= hi[i]; j++) s += a[j];
        out[i] = s;
    }
}
"""

C_GOOD = r"""
#include <stddef.h>
#include <stdlib.h>
void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi,
                size_t q, long long *out) {
    long long *pre = malloc((n + 1) * sizeof *pre);
    if (!pre) return;
    pre[0] = 0;
    for (size_t i = 0; i < n; i++) pre[i + 1] = pre[i] + a[i];
    for (size_t i = 0; i < q; i++) out[i] = pre[hi[i] + 1] - pre[lo[i]];
    free(pre);
}
"""

PY_NAIVE = '''
def dedupe(items):
    out = []
    for x in items:
        if x not in out:
            out.append(x)
    return out
'''

PY_GOOD = '''
def dedupe(items):
    seen = set()
    out = []
    for x in items:
        if x not in seen:
            seen.add(x)
            out.append(x)
    return out
'''

SH_NAIVE = r"""
top_freq() {
    local f=$1 v
    for v in $(sort -u "$f"); do
        printf '%s %s\n' "$(grep -c -x -- "$v" "$f")" "$v"
    done | sort -k1,1nr -k2,2 | head -10
}
"""

SH_GOOD = r"""
top_freq() {
    sort "$1" | uniq -c | sort -k1,1nr -k2,2 | head -10 | awk '{print $1, $2}'
}
"""

CASES = [
    ("range_sums", C_NAIVE, C_GOOD),
    ("dedupe", PY_NAIVE, PY_GOOD),
    ("top_freq", SH_NAIVE, SH_GOOD),
]


def run(task, code):
    with tempfile.TemporaryDirectory() as td:
        return eval_perf.measure(task, code, Path(td))


def main() -> int:
    tasks = {t["name"]: t for t in eval_perf.TASKS}
    bad = 0
    for name, naive, good in CASES:
        task = tasks[name]
        ns, nms, nnote = run(task, naive)
        gs, gms, gnote = run(task, good)
        print(f"{task['lang']:7} {name}")
        print(f"    naive: {ns:16} {nms if nms is not None else '—':>12} ms  {nnote[:60]}")
        print(f"    good:  {gs:16} {gms if gms is not None else '—':>12} ms  {gnote[:60]}")
        if gs != "pass":
            print("    BROKEN: the good solution does not pass")
            bad += 1
            continue
        if ns == "pass" and nms:
            ratio = nms / gms if gms else 0
            print(f"    spread: {ratio:,.0f}x")
            if ratio < 10:
                print("    TOO NARROW: naive is within 10x, this measures nothing")
                bad += 1
        elif ns == "too_slow":
            print(f"    spread: naive exceeds the {eval_perf.RUN_TIMEOUT:.0f}s timeout")
        else:
            print(f"    NOTE: naive did not run cleanly ({ns}) — correctness should be easy")
            bad += 1
    return 1 if bad else 0


if __name__ == "__main__":
    raise SystemExit(main())
