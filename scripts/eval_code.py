#!/usr/bin/env python3
"""Verifiable C coding eval: does the model's code compile and pass tests?

Objective quality signal — no LLM judge. Each task asks for a C function,
extracts the code, compiles it with a hidden test harness, runs the tests.

Usage:
  scripts/eval_code.py --target both          # mlx (8080) + ornith (8082)
  scripts/eval_code.py --target ornith --json
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).parent))
from bench import TARGETS, complete_openai  # noqa: E402

MAX_TOKENS = 1024
MAX_TOKENS_HARMONY = 4096  # gpt-oss analysis channel eats budget
RUN_TIMEOUT = 5.0
HARMONY_TARGETS = {"gptoss"}
THINKING_TARGETS = {"deepseek-32b"}


def strip_harmony(reply: str) -> str:
    """gpt-oss leaks <|channel|>analysis<|message|>... before the final answer."""
    marker = "<|channel|>final<|message|>"
    if marker in reply:
        return reply.split(marker, 1)[1]
    return reply

TASKS: list[dict[str, str]] = [
    {
        "name": "reverse_string",
        "sig": "void reverse_string(char *s)",
        "prompt": "Reverses s in place.",
        "test": r"""
#include <stdio.h>
#include <string.h>
void reverse_string(char *s);
static int fails = 0;
static void check(const char *in, const char *want) {
    char buf[256]; strcpy(buf, in); reverse_string(buf);
    if (strcmp(buf, want)) { printf("FAIL \"%s\" -> \"%s\" want \"%s\"\n", in, buf, want); fails++; }
}
int main(void) {
    check("hello", "olleh"); check("", ""); check("a", "a");
    check("ab", "ba"); check("racecar", "racecar"); check("Hello World", "dlroW olleH");
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "is_prime",
        "sig": "int is_prime(int n)",
        "prompt": "Returns 1 if n is prime, 0 otherwise. n <= 1 is not prime.",
        "test": r"""
#include <stdio.h>
int is_prime(int n);
static int fails = 0;
static void check(int n, int want) {
    int got = is_prime(n);
    if ((got != 0) != (want != 0)) { printf("FAIL is_prime(%d)=%d want %d\n", n, got, want); fails++; }
}
int main(void) {
    check(2,1); check(3,1); check(5,1); check(7,1); check(97,1); check(7919,1);
    check(0,0); check(1,0); check(-3,0); check(4,0); check(9,0); check(91,0); check(100,0);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "fizzbuzz",
        "sig": "const char *fizzbuzz(int n)",
        "prompt": 'Returns "FizzBuzz" if n divisible by 15, "Fizz" by 3, "Buzz" by 5, else the decimal string of n. A static buffer is fine.',
        "test": r"""
#include <stdio.h>
#include <string.h>
const char *fizzbuzz(int n);
static int fails = 0;
static void check(int n, const char *want) {
    const char *got = fizzbuzz(n);
    if (strcmp(got, want)) { printf("FAIL fizzbuzz(%d)=\"%s\" want \"%s\"\n", n, got, want); fails++; }
}
int main(void) {
    check(1,"1"); check(3,"Fizz"); check(5,"Buzz"); check(15,"FizzBuzz");
    check(7,"7"); check(30,"FizzBuzz"); check(9,"Fizz"); check(100,"Buzz"); check(-3,"Fizz");
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "binary_search",
        "sig": "int binary_search(const int *a, int n, int key)",
        "prompt": "a is sorted ascending, n elements. Returns the index of key, or -1 if absent.",
        "test": r"""
#include <stdio.h>
int binary_search(const int *a, int n, int key);
static int fails = 0;
static void check(const int *a, int n, int key, int want) {
    int got = binary_search(a, n, key);
    if (got != want) { printf("FAIL bsearch key=%d got %d want %d\n", key, got, want); fails++; }
}
int main(void) {
    int a[] = {1,3,5,7,9,11,13}; int one[] = {42};
    check(a,7,1,0); check(a,7,13,6); check(a,7,7,3); check(a,7,4,-1);
    check(a,7,0,-1); check(a,7,99,-1); check(one,1,42,0); check(one,1,1,-1); check(a,0,5,-1);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "count_words",
        "sig": "int count_words(const char *s)",
        "prompt": "Counts words in s. Words are separated by one or more spaces; leading/trailing spaces possible.",
        "test": r"""
#include <stdio.h>
int count_words(const char *s);
static int fails = 0;
static void check(const char *s, int want) {
    int got = count_words(s);
    if (got != want) { printf("FAIL count_words(\"%s\")=%d want %d\n", s, got, want); fails++; }
}
int main(void) {
    check("hello",1); check("hello world",2); check("the quick  brown fox",4);
    check("",0); check("   ",0); check("  pad  ",1); check("a b c d e",5);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "max_subarray",
        "sig": "int max_subarray(const int *a, int n)",
        "prompt": "Returns the maximum sum of any non-empty contiguous subarray (Kadane). n >= 1.",
        "test": r"""
#include <stdio.h>
int max_subarray(const int *a, int n);
static int fails = 0;
static void check(const int *a, int n, int want) {
    int got = max_subarray(a, n);
    if (got != want) { printf("FAIL max_subarray got %d want %d\n", got, want); fails++; }
}
int main(void) {
    int t1[] = {-2,1,-3,4,-1,2,1,-5,4}; int t2[] = {-3,-1,-2};
    int t3[] = {5}; int t4[] = {1,2,3,4}; int t5[] = {2,-1,2};
    check(t1,9,6); check(t2,3,-1); check(t3,1,5); check(t4,4,10); check(t5,3,3);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "itoa",
        "sig": "void itoa(int value, char *buf)",
        "prompt": "Writes the decimal string of value into buf (NUL-terminated). Must handle 0 and negatives, including INT_MIN.",
        "test": r"""
#include <stdio.h>
#include <string.h>
#include <limits.h>
void itoa(int value, char *buf);
static int fails = 0;
static void check(int v) {
    char buf[32], want[32];
    itoa(v, buf); snprintf(want, sizeof want, "%d", v);
    if (strcmp(buf, want)) { printf("FAIL itoa(%d)=\"%s\" want \"%s\"\n", v, buf, want); fails++; }
}
int main(void) {
    check(0); check(7); check(-7); check(12345); check(-98765); check(INT_MAX); check(INT_MIN);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "stack",
        "sig": "void stack_init(Stack *s); int stack_push(Stack *s, int v); int stack_pop(Stack *s, int *out)",
        "prompt": "Fixed-capacity int stack (16). Include this exact definition in your code: typedef struct { int data[16]; int top; } Stack; push returns 0 on success, -1 when full; pop returns 0 on success, -1 when empty.",
        "test": r"""
#include <stdio.h>
typedef struct { int data[16]; int top; } Stack;
void stack_init(Stack *s); int stack_push(Stack *s, int v); int stack_pop(Stack *s, int *out);
static int fails = 0;
int main(void) {
    Stack s; int v = -999; stack_init(&s);
    if (stack_pop(&s, &v) != -1) { printf("FAIL pop empty\n"); fails++; }
    for (int i = 0; i < 16; i++) if (stack_push(&s, i) != 0) { printf("FAIL push %d\n", i); fails++; }
    if (stack_push(&s, 16) != -1) { printf("FAIL push full\n"); fails++; }
    for (int i = 15; i >= 0; i--) {
        if (stack_pop(&s, &v) != 0 || v != i) { printf("FAIL pop got %d want %d\n", v, i); fails++; }
    }
    if (stack_pop(&s, &v) != -1) { printf("FAIL pop empty2\n"); fails++; }
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "popcount",
        "sig": "unsigned popcount(unsigned x)",
        "prompt": "Returns the number of set bits in x.",
        "test": r"""
#include <stdio.h>
unsigned popcount(unsigned x);
static int fails = 0;
static void check(unsigned x, unsigned want) {
    unsigned got = popcount(x);
    if (got != want) { printf("FAIL popcount(%u)=%u want %u\n", x, got, want); fails++; }
}
int main(void) {
    check(0,0); check(1,1); check(0xFFFFFFFFu,32); check(0xF0F0F0F0u,16);
    check(7,3); check(0x80000000u,1); check(123456789u,16);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "rot13",
        "sig": "void rot13(char *s)",
        "prompt": "Applies ROT13 to s in place: letters rotate by 13, all other characters unchanged.",
        "test": r"""
#include <stdio.h>
#include <string.h>
void rot13(char *s);
static int fails = 0;
static void check(const char *in, const char *want) {
    char buf[256]; strcpy(buf, in); rot13(buf);
    if (strcmp(buf, want)) { printf("FAIL rot13(\"%s\")=\"%s\" want \"%s\"\n", in, buf, want); fails++; }
}
int main(void) {
    check("hello", "uryyb"); check("uryyb", "hello"); check("ABC xyz", "NOP klm");
    check("", ""); check("123!", "123!"); check("Zebra", "Mroen");
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "parse_csv_ints",
        "sig": "int parse_csv_ints(const char *s, int *out, int max)",
        "prompt": "Parses comma-separated integers from s into out (at most max). Returns the count stored. Input is well-formed (digits, optional leading minus, commas).",
        "test": r"""
#include <stdio.h>
int parse_csv_ints(const char *s, int *out, int max);
static int fails = 0;
static void check(const char *s, int max, const int *want, int wantn) {
    int out[16]; int got = parse_csv_ints(s, out, max);
    if (got != wantn) { printf("FAIL parse(\"%s\") count %d want %d\n", s, got, wantn); fails++; return; }
    for (int i = 0; i < wantn; i++)
        if (out[i] != want[i]) { printf("FAIL parse(\"%s\")[%d]=%d want %d\n", s, i, out[i], want[i]); fails++; return; }
}
int main(void) {
    int w1[] = {10,20,30}; int w2[] = {-5}; int w3[] = {1,2};
    check("10,20,30", 16, w1, 3); check("-5", 16, w2, 1);
    check("1,2,3,4,5", 2, w3, 2); check("", 16, 0, 0);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "bswap32",
        "sig": "uint32_t bswap32(uint32_t x)",
        "prompt": "Returns x with its four bytes reversed (endian swap). Include <stdint.h> yourself.",
        "test": r"""
#include <stdio.h>
#include <stdint.h>
uint32_t bswap32(uint32_t x);
static int fails = 0;
static void check(uint32_t x, uint32_t want) {
    uint32_t got = bswap32(x);
    if (got != want) { printf("FAIL bswap32(0x%08X)=0x%08X want 0x%08X\n", x, got, want); fails++; }
}
int main(void) {
    check(0x12345678u, 0x78563412u); check(0x00000001u, 0x01000000u);
    check(0u, 0u); check(0xAABBCCDDu, 0xDDCCBBAAu); check(0xFF0000FFu, 0xFF0000FFu);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "trim",
        "sig": "void trim(char *s)",
        "prompt": "Strips leading and trailing whitespace (space, tab, newline) from s in place.",
        "test": r"""
#include <stdio.h>
#include <string.h>
void trim(char *s);
static int fails = 0;
static void check(const char *in, const char *want) {
    char buf[256]; strcpy(buf, in); trim(buf);
    if (strcmp(buf, want)) { printf("FAIL trim(\"%s\")=\"%s\" want \"%s\"\n", in, buf, want); fails++; }
}
int main(void) {
    check("  hello  ", "hello"); check("hello", "hello"); check("", "");
    check("   ", ""); check("\t hi \n", "hi"); check("a b", "a b");
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "cmp_desc",
        "sig": "int cmp_desc(const void *a, const void *b)",
        "prompt": "qsort comparator ordering int values descending. Must be safe against overflow.",
        "test": r"""
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
int cmp_desc(const void *a, const void *b);
int main(void) {
    int a[] = {3, -1, 7, 0, INT_MAX, INT_MIN, 7};
    int want[] = {INT_MAX, 7, 7, 3, 0, -1, INT_MIN};
    qsort(a, 7, sizeof(int), cmp_desc);
    for (int i = 0; i < 7; i++)
        if (a[i] != want[i]) { printf("FAIL sorted[%d]=%d want %d\n", i, a[i], want[i]); return 1; }
    printf("PASS\n");
    return 0;
}
""",
    },
    {
        "name": "ring",
        "sig": "void ring_init(Ring *r); int ring_push(Ring *r, int v); int ring_pop(Ring *r, int *out)",
        "prompt": "Circular buffer, capacity 8, FIFO. Include this exact definition in your code: typedef struct { int data[8]; int head; int count; } Ring; push returns 0 on success, -1 when full; pop returns 0 on success, -1 when empty. Pushes after pops must reuse freed slots (wraparound).",
        "test": r"""
#include <stdio.h>
typedef struct { int data[8]; int head; int count; } Ring;
void ring_init(Ring *r); int ring_push(Ring *r, int v); int ring_pop(Ring *r, int *out);
static int fails = 0;
int main(void) {
    Ring r; int v; ring_init(&r);
    if (ring_pop(&r, &v) != -1) { printf("FAIL pop empty\n"); fails++; }
    for (int i = 0; i < 8; i++) if (ring_push(&r, i) != 0) { printf("FAIL push %d\n", i); fails++; }
    if (ring_push(&r, 99) != -1) { printf("FAIL push full\n"); fails++; }
    for (int i = 0; i < 4; i++) { if (ring_pop(&r, &v) != 0 || v != i) { printf("FAIL pop got %d want %d\n", v, i); fails++; } }
    for (int i = 8; i < 12; i++) if (ring_push(&r, i) != 0) { printf("FAIL wrap push %d\n", i); fails++; }
    for (int i = 4; i < 12; i++) { if (ring_pop(&r, &v) != 0 || v != i) { printf("FAIL wrap pop got %d want %d\n", v, i); fails++; } }
    if (ring_pop(&r, &v) != -1) { printf("FAIL pop empty2\n"); fails++; }
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "atoi_strict",
        "sig": "int atoi_strict(const char *s, int *out)",
        "prompt": "Parses an optionally negative decimal integer from s into out. Returns 0 on success, -1 if s is not exactly one valid integer (empty, junk, trailing chars). No overflow tests.",
        "test": r"""
#include <stdio.h>
int atoi_strict(const char *s, int *out);
static int fails = 0;
static void ok(const char *s, int want) {
    int v = 0;
    if (atoi_strict(s, &v) != 0 || v != want) { printf("FAIL atoi_strict(\"%s\") v=%d want %d\n", s, v, want); fails++; }
}
static void bad(const char *s) {
    int v = 0;
    if (atoi_strict(s, &v) != -1) { printf("FAIL atoi_strict(\"%s\") should fail\n", s); fails++; }
}
int main(void) {
    ok("0",0); ok("42",42); ok("-42",-42); ok("007",7);
    bad(""); bad("abc"); bad("12x"); bad("-"); bad("1.5"); bad(" 12");
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
]


def _sanitize(code: str) -> str:
    """Repair common fence/header mangling seen in wild outputs."""
    out = []
    for line in code.splitlines():
        s = line.strip()
        m = re.match(r"^(?:[a-z]*\s*)?(?:import|include)\s*(<[^>]+>)\s*$", s)
        if m:
            line = f"#include {m.group(1)}"
        elif re.match(r"^<[a-z_]+\.h>\s*$", s):
            line = f"#include {s}"
        out.append(line)
    return "\n".join(out)


def extract_code(text: str, must_contain: str) -> str:
    blocks = re.findall(r"```[A-Za-z0-9+#]*[ \t]*\n(.*?)```", text, re.S)
    if not blocks:
        # malformed fence: language tag glued onto the first code line
        blocks = re.findall(r"```[A-Za-z0-9+#]*(.*?)\n```", text, re.S)
    func = must_contain.split("(")[0].split()[-1]
    for block in blocks:
        if func in block:
            return _sanitize(block.strip())
    if blocks:
        return _sanitize(max(blocks, key=len).strip())
    return _sanitize(text.strip()) if func in text else ""


def grade(task: dict[str, str], code: str, workdir: Path) -> tuple[str, str]:
    if not code:
        return "no_code", ""
    sol = workdir / "solution.c"
    sol.write_text(code)
    (workdir / "test.c").write_text(task["test"])
    # -Dmain=... neutralizes any stray main() in the solution.
    compile_sol = subprocess.run(
        ["cc", "-std=c11", "-O1", "-Wall", "-Dmain=solution_unused_main",
         "-c", "solution.c", "-o", "solution.o"],
        cwd=workdir, capture_output=True, text=True, timeout=60,
    )
    if compile_sol.returncode != 0:
        return "compile_error", compile_sol.stderr.strip().splitlines()[0][:100] if compile_sol.stderr else ""
    link = subprocess.run(
        ["cc", "test.c", "solution.o", "-o", "test_bin"],
        cwd=workdir, capture_output=True, text=True, timeout=60,
    )
    if link.returncode != 0:
        return "link_error", link.stderr.strip().splitlines()[-1][:100] if link.stderr else ""
    try:
        run = subprocess.run(
            ["./test_bin"], cwd=workdir, capture_output=True, text=True, timeout=RUN_TIMEOUT,
        )
    except subprocess.TimeoutExpired:
        return "timeout", ""
    if run.returncode == 0 and "PASS" in run.stdout:
        return "pass", ""
    first_fail = next((l for l in run.stdout.splitlines() if l.startswith("FAIL")), "")
    return "wrong_answer", first_fail[:100]


def eval_target(name: str, timeout: float, trials: int) -> dict[str, Any]:
    cfg = TARGETS[name]
    results: dict[str, list[str]] = {}
    notes: dict[str, str] = {}
    for task in TASKS:
        prompt = (
            f"Implement in C11: `{task['sig']}`. {task['prompt']}\n"
            "Reply with only a C code block. No main function, no tests, no explanation."
        )
        outcomes: list[str] = []
        max_tok = MAX_TOKENS_HARMONY if name in HARMONY_TARGETS else MAX_TOKENS
        if name in THINKING_TARGETS:
            max_tok = MAX_TOKENS_HARMONY
        for trial in range(trials):
            temp = 0.0 if trial == 0 else 0.7
            try:
                reply = complete_openai(
                    port=cfg["port"], model=cfg["model"], prompt=prompt,
                    max_tokens=max_tok, timeout=timeout, temperature=temp,
                )
            except Exception as exc:  # noqa: BLE001
                outcomes.append("http_error")
                notes[task["name"]] = str(exc)[:100]
                continue
            reply = strip_harmony(reply) if name in HARMONY_TARGETS else reply
            if name in THINKING_TARGETS:
                # DeepSeek-R1 distills leak <think>...</think> before the answer.
                if "</think>" in reply:
                    reply = reply.split("</think>", 1)[1]
            code = extract_code(reply, task["sig"])
            with tempfile.TemporaryDirectory() as td:
                status, note = grade(task, code, Path(td))
            outcomes.append(status)
            if status != "pass" and not notes.get(task["name"]):
                notes[task["name"]] = note
        results[task["name"]] = outcomes
        marks = ",".join("✓" if s == "pass" else "✗" for s in outcomes)
        print(f"{name} {task['name']}: [{marks}] {notes.get(task['name'], '')}", flush=True)
    total_pass = sum(sum(1 for s in o if s == "pass") for o in results.values())
    return {
        "target": name,
        "model": cfg["model"],
        "passed": total_pass,
        "total": len(TASKS) * trials,
        "trials": trials,
        "results": results,
        "notes": notes,
    }


def print_table(rows: list[dict[str, Any]]) -> None:
    names = [r["target"] for r in rows]
    print()
    header = f"{'task':<16}" + "".join(f"{n:>10}" for n in names)
    print(header)
    print("-" * len(header))
    for task in TASKS:
        line = f"{task['name']:<16}"
        for r in rows:
            outcomes = r["results"].get(task["name"], [])
            cell = f"{sum(1 for s in outcomes if s == 'pass')}/{len(outcomes)}"
            line += f"{cell:>10}"
        print(line)
    print("-" * len(header))
    print(f"{'pass rate':<16}" + "".join(f"{r['passed']}/{r['total']:<8}" for r in rows))


def main() -> None:
    parser = argparse.ArgumentParser(description="Verifiable C coding eval")
    parser.add_argument("--target", choices=tuple(TARGETS) + ("both", "all"), default="both")
    parser.add_argument("--trials", type=int, default=1)
    parser.add_argument("--timeout", type=float, default=180.0)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    if args.target == "all":
        names = [n for n, c in TARGETS.items() if c["kind"] == "openai"]
    else:
        names = ["mlx", "ornith"] if args.target == "both" else [args.target]
    rows = [eval_target(name, args.timeout, args.trials) for name in names]
    if args.json:
        print(json.dumps(rows, indent=2))
    else:
        print_table(rows)


if __name__ == "__main__":
    main()
