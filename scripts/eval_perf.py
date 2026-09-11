#!/usr/bin/env python3
"""Performance eval: is the generated code actually fast?

Every other eval here asks whether the code is correct. This one asks how long it
takes to run. The three tasks are deliberately easy to get *right* — a beginner's
answer passes the correctness check — so the only thing that varies is whether the
model thought about complexity at all.

Each task runs in two prompt variants:
  silent  no mention of performance, which is what an agent loop actually sends
  told    the prompt says the running time will be measured

The gap between the two answers "does this model need to be told?".

Timing protocol: correctness is checked on a small input first; only then is the
solution run on a large hidden input, once to warm up and then three times, taking
the minimum. Measured spread on an idle machine is about 3%, far below the
algorithmic differences being measured. C is always compiled -O2 so we are
comparing algorithms, not the model's choice of compiler flags.
"""
from __future__ import annotations

import argparse
import datetime as dt
import json
import random
import re
import subprocess
import sys
import tempfile
import time
from collections import Counter
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))

import eval_bash
import eval_code
import eval_python
from bench import TARGETS, complete_openai_full

RESULTS = Path(__file__).resolve().parent.parent / "results" / "perf"
PY = str(Path(__file__).resolve().parent.parent / ".venv" / "bin" / "python")
TIMED_RUNS = 3        # measured runs after one warm-up
RUN_TIMEOUT = 90.0    # a solution slower than this is recorded as too_slow

# ---------------------------------------------------------------- C: range sums

C_BENCH = r"""
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi,
                size_t q, long long *out);

static unsigned long long rs = 88172645463325252ULL;
static unsigned long long xr(void) {
    rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return rs;
}

int main(void) {
    const size_t n = 100000, q = 100000;
    int *a = malloc(n * sizeof *a);
    size_t *lo = malloc(q * sizeof *lo), *hi = malloc(q * sizeof *hi);
    long long *out = calloc(q, sizeof *out);
    if (!a || !lo || !hi || !out) { printf("BAD alloc\n"); return 1; }
    for (size_t i = 0; i < n; i++) a[i] = (int)(xr() % 2001) - 1000;
    for (size_t i = 0; i < q; i++) {
        size_t x = xr() % n, y = xr() % n;
        lo[i] = x < y ? x : y;
        hi[i] = x < y ? y : x;
    }
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    range_sums(a, n, lo, hi, q, out);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double ms = (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1e6;

    /* spot-check 20 queries directly; cheap, and no hardcoded constants */
    for (size_t k = 0; k < 20; k++) {
        size_t i = (size_t)(xr() % q);
        long long want = 0;
        for (size_t j = lo[i]; j <= hi[i]; j++) want += a[j];
        if (want != out[i]) { printf("BAD query %zu\n", i); return 1; }
    }
    printf("OK\nMS %.3f\n", ms);
    return 0;
}
"""

C_CORRECT = r"""
#include <stdio.h>
#include <stddef.h>
void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi,
                size_t q, long long *out);
int main(void) {
    int a[8] = {5, -2, 7, 0, 3, 3, -1, 4};
    size_t lo[6] = {0, 2, 7, 0, 3, 1};
    size_t hi[6] = {0, 4, 7, 7, 5, 3};
    long long want[6] = {5, 10, 4, 19, 6, 5};
    long long out[6] = {0};
    range_sums(a, 8, lo, hi, 6, out);
    for (int i = 0; i < 6; i++)
        if (out[i] != want[i]) {
            printf("FAIL q%d = %lld, want %lld\n", i, out[i], want[i]);
            return 1;
        }
    printf("PASS\n");
    return 0;
}
"""

# ------------------------------------------------------------ Python: dedupe

PY_BENCH = '''
import random, time
from solution import dedupe

rnd = random.Random(20260910)
items = [f"item{rnd.randrange(10000)}" for _ in range(20000)]

t0 = time.perf_counter()
got = dedupe(items)
ms = (time.perf_counter() - t0) * 1000

seen, want = set(), []
for x in items:
    if x not in seen:
        seen.add(x)
        want.append(x)
if list(got) != want:
    print("BAD result")
    raise SystemExit(1)
print("OK")
print(f"MS {ms:.3f}")
'''

PY_CORRECT = '''
from solution import dedupe
assert dedupe([]) == []
assert dedupe([1, 2, 3]) == [1, 2, 3]
assert dedupe([1, 1, 1]) == [1]
assert dedupe([3, 1, 3, 2, 1]) == [3, 1, 2]
assert dedupe(["b", "a", "b", "c", "a"]) == ["b", "a", "c"]
assert dedupe([0, False, 1]) in ([0, 1], [0, False, 1])
print("PASS")
'''

# ------------------------------------------------------- Bash: top frequencies

SH_BENCH = r"""
source solution.sh
top_freq data.txt > got.txt 2>/dev/null
if diff -q want.txt got.txt >/dev/null 2>&1; then echo OK; else echo "BAD output"; exit 1; fi
"""

# Every count is distinct, so the ordering is unambiguous and no tie-break rule
# has to be guessed. v11/v12 exist only so that `head -10` is actually exercised.
SH_CORRECT = r"""
source solution.sh
{
  n=11
  for v in v01 v02 v03 v04 v05 v06 v07 v08 v09 v10; do
    for ((i = 0; i < n; i++)); do printf '%s\n' "$v"; done
    n=$((n - 1))
  done
  printf 'v11\nv12\n'
} > small.txt
printf '11 v01\n10 v02\n9 v03\n8 v04\n7 v05\n6 v06\n5 v07\n4 v08\n3 v09\n2 v10\n' > want.txt
top_freq small.txt > got.txt 2>/dev/null
if ! diff -q want.txt got.txt >/dev/null 2>&1; then
  echo "FAIL:"; diff want.txt got.txt | head -20; exit 1
fi
echo PASS
"""

TASKS: list[dict[str, Any]] = [
    {
        "name": "range_sums",
        "lang": "c",
        "sig": ("void range_sums(const int *a, size_t n, const size_t *lo, "
                "const size_t *hi, size_t q, long long *out)"),
        "task": ("For each query i, out[i] must be the sum of the elements a[lo[i]] "
                 "through a[hi[i]] inclusive. Every query satisfies lo[i] <= hi[i] < n."),
        "hint": ("It will be called with n and q both in the hundreds of thousands, and its "
                 "running time will be measured."),
    },
    {
        "name": "dedupe",
        "lang": "python",
        "sig": "def dedupe(items):",
        "task": ("Returns a new list with duplicates removed, keeping the first occurrence "
                 "of each item in its original order. All items are hashable."),
        "hint": ("It will be called with lists of tens of thousands of items, and its running "
                 "time will be measured."),
    },
    {
        "name": "top_freq",
        "lang": "bash",
        "sig": "top_freq <file>",
        "task": ("The file holds one value per line. Print the 10 most frequent values, most "
                 "frequent first, one per line, each formatted as the count, a single space, "
                 "then the value."),
        "hint": ("The file will have hundreds of thousands of lines, and the function's "
                 "running time will be measured."),
    },
]

LANG_META = {
    "c": {"label": "C11", "fence": "C", "ext": "c"},
    "python": {"label": "Python", "fence": "Python", "ext": "py"},
    "bash": {"label": "bash", "fence": "bash", "ext": "sh"},
}


def build_prompt(task: dict[str, Any], variant: str) -> str:
    meta = LANG_META[task["lang"]]
    if task["lang"] == "bash":
        head = f"Write a {meta['label']} function `{task['sig']}`."
    else:
        head = f"Implement in {meta['label']}: `{task['sig']}`."
    hint = f" {task['hint']}" if variant == "told" else ""
    tail = {
        "c": "Reply with only a C code block. No main function, no tests, no explanation.",
        "python": "Reply with only a Python code block. No tests, no explanation.",
        "bash": "Reply with only a bash code block. Define the function only, no tests, "
                "no explanation.",
    }[task["lang"]]
    return f"{head} {task['task']}{hint}\n{tail}"


# --------------------------------------------------------------- measurement

def _run(cmd: list[str], cwd: Path, timeout: float) -> subprocess.CompletedProcess | None:
    try:
        return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True,
                              errors="replace", timeout=timeout)
    except subprocess.TimeoutExpired:
        return None


def _parse_ms(stdout: str) -> float | None:
    m = re.search(r"^MS ([0-9.]+)$", stdout, re.M)
    return float(m.group(1)) if m else None


def prepare_bash_data(wd: Path) -> None:
    """200k lines over 500 values.

    The top 12 values get explicit, well-separated counts so the top 10 ordering
    is unambiguous — no model has to guess a tie-break rule. The remaining 488
    values split the rest and land around 250 each, far below the 10th place.
    """
    rnd = random.Random(20260910)
    lines: list[str] = []
    head_counts = [9000 - 500 * i for i in range(12)]   # 9000 down to 3500
    for i, c in enumerate(head_counts):
        lines.extend([f"value{i:03d}"] * c)
    tail = [f"value{i:03d}" for i in range(12, 500)]
    lines.extend(rnd.choices(tail, k=200000 - sum(head_counts)))
    rnd.shuffle(lines)
    (wd / "data.txt").write_text("\n".join(lines) + "\n")

    counts = Counter(lines)
    top = counts.most_common(12)
    if top[9][1] == top[10][1]:
        raise SystemExit("bench data has a tie at the top-10 boundary")
    (wd / "want.txt").write_text("".join(f"{c} {v}\n" for v, c in top[:10]))


def measure(task: dict[str, Any], code: str, wd: Path) -> tuple[str, float | None, str]:
    """Returns (status, best_ms, note). Correctness is checked before timing."""
    lang = task["lang"]
    if not code:
        return "no_code", None, ""

    if lang == "c":
        (wd / "solution.c").write_text(code)
        (wd / "correct.c").write_text(C_CORRECT)
        (wd / "bench.c").write_text(C_BENCH)
        comp = _run(["cc", "-std=c11", "-O2", "-Wall", "-Dmain=solution_unused_main",
                     "-c", "solution.c", "-o", "solution.o"], wd, 60)
        if comp is None or comp.returncode != 0:
            return "compile_error", None, (comp.stderr.strip()[:200] if comp else "timeout")
        for src, out in (("correct.c", "correct_bin"), ("bench.c", "bench_bin")):
            link = _run(["cc", "-O2", src, "solution.o", "-o", out], wd, 60)
            if link is None or link.returncode != 0:
                return "link_error", None, (link.stderr.strip()[-200:] if link else "timeout")
        chk = _run(["./correct_bin"], wd, 30)
        if chk is None or "PASS" not in chk.stdout:
            return "wrong", None, (chk.stdout.strip()[:200] if chk else "timeout")
        cmd = ["./bench_bin"]
    elif lang == "python":
        (wd / "solution.py").write_text(code)
        (wd / "correct.py").write_text(PY_CORRECT)
        (wd / "bench.py").write_text(PY_BENCH)
        chk = _run([PY, "correct.py"], wd, 30)
        if chk is None or "PASS" not in chk.stdout:
            note = (chk.stderr or chk.stdout).strip()[-200:] if chk else "timeout"
            return "wrong", None, note
        cmd = [PY, "bench.py"]
    else:
        (wd / "solution.sh").write_text(code)
        (wd / "correct.sh").write_text(SH_CORRECT)
        (wd / "bench.sh").write_text(SH_BENCH)
        chk = _run(["bash", "correct.sh"], wd, 60)
        if chk is None or "PASS" not in chk.stdout:
            note = ((chk.stdout or "") + (chk.stderr or "")).strip()[-200:] if chk else "timeout"
            return "wrong", None, note
        prepare_bash_data(wd)
        cmd = ["bash", "bench.sh"]

    # one warm-up, then TIMED_RUNS measured; keep the minimum
    times: list[float] = []
    for i in range(TIMED_RUNS + 1):
        t0 = time.perf_counter()
        r = _run(cmd, wd, RUN_TIMEOUT)
        wall = (time.perf_counter() - t0) * 1000
        if r is None:
            return "too_slow", None, f"exceeded {RUN_TIMEOUT:.0f}s"
        if r.returncode != 0 or "OK" not in r.stdout:
            note = ((r.stdout or "") + (r.stderr or "")).strip()[:200]
            return "wrong_on_large", None, note
        if i == 0:
            continue
        # C and Python time the function internally; bash is timed externally
        inner = _parse_ms(r.stdout)
        times.append(inner if inner is not None else wall)
    return "pass", round(min(times), 3), ""


# ------------------------------------------------------------------- driver

def extract(task: dict[str, Any], reply: str) -> str:
    lang = task["lang"]
    if lang == "c":
        return eval_code.extract_code(reply, task["sig"])
    func = task["sig"].split("(")[0].replace("def ", "").strip().split()[0]
    if lang == "python":
        return eval_python.extract_python(reply, func)
    return eval_bash.extract_bash(reply, func)


def run_target(target: str, trials: int, timeout: float,
               dump_dir: str | None) -> dict[str, Any]:
    cfg = TARGETS[target]
    max_tok = (eval_code.MAX_TOKENS_HARMONY
               if target in eval_code.HARMONY_TARGETS or target in eval_code.THINKING_TARGETS
               else eval_code.MAX_TOKENS)
    out: dict[str, Any] = {"target": target, "model": cfg["model"], "variants": {}}

    for variant in ("silent", "told"):
        vres: dict[str, Any] = {}
        for task in TASKS:
            prompt = build_prompt(task, variant)
            statuses, mss = [], []
            note = ""
            for trial in range(trials):
                temp = 0.0 if trial == 0 else 0.7
                try:
                    resp = complete_openai_full(
                        port=cfg["port"], model=cfg["model"], prompt=prompt,
                        max_tokens=max_tok, timeout=timeout, temperature=temp)
                except Exception as exc:  # noqa: BLE001
                    statuses.append("http_error")
                    note = note or str(exc)[:120]
                    continue
                reply = resp["text"]
                if target in eval_code.HARMONY_TARGETS:
                    reply = eval_code.strip_harmony(reply)
                if target in eval_code.THINKING_TARGETS and "</think>" in reply:
                    reply = reply.split("</think>", 1)[1]
                code = extract(task, reply)
                with tempfile.TemporaryDirectory() as td:
                    status, ms, n = measure(task, code, Path(td))
                statuses.append(status)
                if ms is not None:
                    mss.append(ms)
                if status != "pass":
                    note = note or n
                    if dump_dir and code:
                        d = Path(dump_dir)
                        d.mkdir(parents=True, exist_ok=True)
                        ext = LANG_META[task["lang"]]["ext"]
                        (d / f"{target}-{task['name']}-{variant}-t{trial}.{ext}").write_text(code)
            vres[task["name"]] = {
                "statuses": statuses,
                "ms": mss,
                "best_ms": min(mss) if mss else None,
                "median_ms": round(sorted(mss)[len(mss) // 2], 3) if mss else None,
                "correct": sum(1 for s in statuses if s == "pass"),
                "trials": len(statuses),
                "note": note,
            }
            best = vres[task["name"]]["best_ms"]
            print(f"  {target} {variant:6} {task['name']:11} "
                  f"{vres[task['name']]['correct']}/{len(statuses)} correct, "
                  f"best {best if best is not None else '—'} ms "
                  f"{'' if best is not None else statuses}", file=sys.stderr)
        out["variants"][variant] = vres
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description="Performance eval: how fast is the code?")
    ap.add_argument("--target", choices=tuple(TARGETS), required=True)
    ap.add_argument("--trials", type=int, default=3)
    ap.add_argument("--timeout", type=float, default=600.0)
    ap.add_argument("--dump-failures", metavar="DIR", default=None)
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    started = dt.datetime.now()
    doc = run_target(args.target, args.trials, args.timeout, args.dump_failures)
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
