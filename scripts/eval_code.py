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
from bench import TARGETS, complete_openai, complete_openai_full  # noqa: E402

MAX_TOKENS = 1024
MAX_TOKENS_HARMONY = 4096  # gpt-oss analysis channel eats budget
MAX_TOKENS_BRUTAL = 4096        # brutal tasks are long; a 1024 cap scores truncation
MAX_TOKENS_BRUTAL_HARMONY = 16384
RUN_TIMEOUT = 5.0
PROMPT_TEMPLATE = (
    "Implement in C11: `{sig}`. {prompt}\n"
    "Reply with only a C code block. No main function, no tests, no explanation."
)


def build_prompt(task: dict[str, str]) -> str:
    """The exact instruction sent to the model. eval_repair.py and the HTML report
    both call this, so what the report displays cannot drift from what was asked."""
    return PROMPT_TEMPLATE.format(sig=task["sig"], prompt=task["prompt"])


HARMONY_TARGETS = {"gptoss", "gptoss120"}
THINKING_TARGETS = {"deepseek-32b", "qwen3-30b"}


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
    {
        "name": "glob_match",
        "sig": "int glob_match(const char *pat, const char *str)",
        "prompt": ("Returns 1 if str matches the shell-style pattern pat, else 0. "
                   "'*' matches any sequence (including empty), '?' matches exactly one "
                   "character. All other characters match literally. The whole string must match."),
        "hard": "1",
        "test": r"""
#include <stdio.h>
int glob_match(const char *pat, const char *str);
static int fails = 0;
static void ok(const char *p, const char *s) {
    if (glob_match(p, s) != 1) { printf("FAIL \"%s\" should match \"%s\"\n", s, p); fails++; }
}
static void no(const char *p, const char *s) {
    if (glob_match(p, s) != 0) { printf("FAIL \"%s\" should NOT match \"%s\"\n", s, p); fails++; }
}
int main(void) {
    ok("abc", "abc"); ok("*", ""); ok("*", "anything"); ok("a*", "abc"); ok("*c", "abc");
    ok("a*c", "abc"); ok("a*c", "ac"); ok("a?c", "abc"); ok("a*c*d", "abcd");
    ok("*.*", "file.txt"); ok("a**b", "ab"); ok("?", "x");
    no("a?c", "ac"); no("a?c", "abbc"); no("abc", "abcd"); no("*.c", "file.h");
    no("", "a"); no("a", ""); no("?a", "a");
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "rpn_eval",
        "sig": "double rpn_eval(const char *expr)",
        "prompt": ("Evaluates a space-separated Reverse Polish Notation expression of "
                   "non-negative integers and operators + - * /. Returns the result as a "
                   "double. Input is always valid and non-empty."),
        "hard": "1",
        "test": r"""
#include <stdio.h>
#include <math.h>
double rpn_eval(const char *expr);
static int fails = 0;
static void ok(const char *e, double want) {
    double got = rpn_eval(e);
    if (fabs(got - want) > 1e-9) { printf("FAIL \"%s\" = %f want %f\n", e, got, want); fails++; }
}
int main(void) {
    ok("42", 42); ok("3 4 +", 7); ok("10 3 -", 7); ok("6 2 /", 3);
    ok("10 3 /", 10.0/3.0); ok("5 1 2 + 4 * + 3 -", 14); ok("2 3 4 * +", 14);
    ok("3 4 2 * + 1 5 - /", (3.0 + 4.0 * 2.0) / (1.0 - 5.0));
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "arena_alloc",
        "sig": "void *arena_alloc(size_t n)",
        "prompt": ("Write a complete fixed-buffer allocator, four functions in one code block:\n"
                   "void arena_init(void *buf, size_t size); void *arena_alloc(size_t n); "
                   "void arena_free(void *p); void *arena_realloc(void *p, size_t n);\n"
                   "arena_init hands the allocator a buffer and resets all state. All bookkeeping "
                   "must live inside that buffer; do not call malloc. arena_alloc returns memory "
                   "suitably aligned for any type (_Alignof(max_align_t)), or NULL if the request "
                   "cannot be satisfied; arena_alloc(0) returns NULL. arena_free(NULL) is a no-op. "
                   "Freed blocks must be reusable, and a freed block must merge with a free "
                   "neighbour on either side so that later large requests can use the combined "
                   "space. arena_realloc behaves like realloc, and when the block immediately "
                   "after p is free and large enough it must grow into that neighbour and return "
                   "p unchanged rather than moving the data. Contents up to the smaller of the "
                   "old and new sizes are preserved."),
        "brutal": "1",
        "test": r"""
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
void arena_init(void *buf, size_t size);
void *arena_alloc(size_t n);
void arena_free(void *p);
void *arena_realloc(void *p, size_t n);

static int fails = 0;
static void bad(const char *msg) { printf("FAIL %s\n", msg); fails++; }
static _Alignas(max_align_t) unsigned char pool[8192];

/* fills the arena with 64-byte blocks, returns how many fit */
static int fill(void **ps, int cap) {
    int k = 0;
    void *p;
    while (k < cap && (p = arena_alloc(64)) != NULL) ps[k++] = p;
    return k;
}

int main(void) {
    void *ps[256];
    int k, i;

    /* alignment and basic isolation */
    arena_init(pool, sizeof pool);
    void *a = arena_alloc(1), *b = arena_alloc(64), *c = arena_alloc(37);
    if (!a || !b || !c) bad("basic allocations returned NULL");
    else {
        if ((uintptr_t)a % _Alignof(max_align_t) ||
            (uintptr_t)b % _Alignof(max_align_t) ||
            (uintptr_t)c % _Alignof(max_align_t)) bad("pointer is not max_align_t aligned");
        memset(b, 0xAB, 64); memset(c, 0xCD, 37);
        unsigned char *bb = b;
        for (i = 0; i < 64; i++) if (bb[i] != 0xAB) { bad("block b was corrupted by c"); break; }
    }
    if (arena_alloc(0) != NULL) bad("arena_alloc(0) should return NULL");
    if (arena_alloc(sizeof pool * 4) != NULL) bad("oversized request should return NULL");
    arena_free(NULL);

    /* exhaustion is graceful */
    arena_init(pool, sizeof pool);
    k = fill(ps, 256);
    if (k < 8) bad("arena holds implausibly few 64-byte blocks");

    /* freeing everything must restore one large free region */
    for (i = 0; i < k; i++) arena_free(ps[i]);
    if (arena_alloc(64 * (k / 2)) == NULL) bad("free blocks were never merged");

    /* merging with the following neighbour */
    arena_init(pool, sizeof pool);
    k = fill(ps, 256);
    arena_free(ps[2]); arena_free(ps[3]);
    if (arena_alloc(128) == NULL) bad("did not merge with the following free block");

    /* merging with the preceding neighbour */
    arena_init(pool, sizeof pool);
    k = fill(ps, 256);
    arena_free(ps[5]); arena_free(ps[4]);
    if (arena_alloc(128) == NULL) bad("did not merge with the preceding free block");

    /* realloc must grow into the free neighbour instead of moving */
    arena_init(pool, sizeof pool);
    k = fill(ps, 256);
    memset(ps[2], 0x5A, 64);
    arena_free(ps[3]);
    void *grown = arena_realloc(ps[2], 128);
    if (grown == NULL) bad("realloc could not grow into the adjacent free block");
    else {
        if (grown != ps[2]) bad("realloc moved the block instead of extending in place");
        unsigned char *g = grown;
        for (i = 0; i < 64; i++) if (g[i] != 0x5A) { bad("realloc lost the original contents"); break; }
    }

    /* realloc edge cases */
    arena_init(pool, sizeof pool);
    void *r = arena_realloc(NULL, 32);
    if (r == NULL) bad("realloc(NULL, n) should allocate");
    memset(r, 0x11, 32);
    if (arena_realloc(r, 0) != NULL) bad("realloc(p, 0) should free and return NULL");

    /* shrinking releases the tail for reuse */
    arena_init(pool, sizeof pool);
    void *big = arena_alloc(256);
    if (!big) bad("could not allocate 256 bytes");
    else {
        memset(big, 0x77, 256);
        void *small = arena_realloc(big, 64);
        if (small != big) bad("shrinking realloc should keep the same pointer");
        unsigned char *s = small;
        for (i = 0; i < 64; i++) if (s[i] != 0x77) { bad("shrink lost the kept prefix"); break; }
    }

    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "utf8_next",
        "sig": "int utf8_next(const unsigned char *s, size_t n, uint32_t *cp)",
        "prompt": ("Decodes the first UTF-8 character in the n bytes at s. On success writes the "
                   "code point to *cp and returns how many bytes it consumed (1-4). Returns -1 "
                   "if the input is not strictly valid UTF-8, leaving *cp untouched. Strict "
                   "means rejecting: overlong encodings (any code point not using its shortest "
                   "form), UTF-16 surrogates U+D800-U+DFFF, anything above U+10FFFF, the "
                   "obsolete 5- and 6-byte forms, a continuation byte in the leading position, "
                   "bad continuation bytes, and sequences truncated by n. Needs <stdint.h>."),
        "brutal": "1",
        "test": r"""
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
int utf8_next(const unsigned char *s, size_t n, uint32_t *cp);
static int fails = 0;
static void ok(const char *label, const unsigned char *s, size_t n,
               int wantlen, uint32_t wantcp) {
    uint32_t cp = 0xFFFFFFFFu;
    int got = utf8_next(s, n, &cp);
    if (got != wantlen || (wantlen > 0 && cp != wantcp)) {
        printf("FAIL %s: returned %d cp=U+%04X, want %d", label, got, cp, wantlen);
        if (wantlen > 0) printf(" cp=U+%04X", wantcp);
        printf("\n");
        fails++;
    }
}
int main(void) {
    /* valid, including the boundary values a too-strict decoder rejects */
    ok("ASCII A",        (const unsigned char *)"\x41", 1, 1, 0x41);
    ok("NUL",            (const unsigned char *)"\x00", 1, 1, 0x00);
    ok("U+0080 min 2b",  (const unsigned char *)"\xC2\x80", 2, 2, 0x80);
    ok("U+07FF max 2b",  (const unsigned char *)"\xDF\xBF", 2, 2, 0x7FF);
    ok("U+0800 min 3b",  (const unsigned char *)"\xE0\xA0\x80", 3, 3, 0x800);
    ok("U+D7FF pre-sur", (const unsigned char *)"\xED\x9F\xBF", 3, 3, 0xD7FF);
    ok("U+E000 post-sur",(const unsigned char *)"\xEE\x80\x80", 3, 3, 0xE000);
    ok("U+FFFF max 3b",  (const unsigned char *)"\xEF\xBF\xBF", 3, 3, 0xFFFF);
    ok("U+10000 min 4b", (const unsigned char *)"\xF0\x90\x80\x80", 4, 4, 0x10000);
    ok("U+10FFFF max",   (const unsigned char *)"\xF4\x8F\xBF\xBF", 4, 4, 0x10FFFF);
    ok("euro sign",      (const unsigned char *)"\xE2\x82\xAC", 3, 3, 0x20AC);
    ok("emoji",          (const unsigned char *)"\xF0\x9F\x98\x80", 4, 4, 0x1F600);
    ok("stops at first", (const unsigned char *)"\x41\x42\x43", 3, 1, 0x41);

    /* overlong encodings */
    ok("overlong NUL 2b",  (const unsigned char *)"\xC0\x80", 2, -1, 0);
    ok("overlong C1",      (const unsigned char *)"\xC1\xBF", 2, -1, 0);
    ok("overlong 3b",      (const unsigned char *)"\xE0\x80\x80", 3, -1, 0);
    ok("overlong 3b max",  (const unsigned char *)"\xE0\x9F\xBF", 3, -1, 0);
    ok("overlong 4b",      (const unsigned char *)"\xF0\x80\x80\x80", 4, -1, 0);
    ok("overlong 4b max",  (const unsigned char *)"\xF0\x8F\xBF\xBF", 4, -1, 0);
    /* surrogates */
    ok("surrogate D800",   (const unsigned char *)"\xED\xA0\x80", 3, -1, 0);
    ok("surrogate DFFF",   (const unsigned char *)"\xED\xBF\xBF", 3, -1, 0);
    /* out of range */
    ok("above 10FFFF",     (const unsigned char *)"\xF4\x90\x80\x80", 4, -1, 0);
    ok("lead F5",          (const unsigned char *)"\xF5\x80\x80\x80", 4, -1, 0);
    /* obsolete long forms and invalid leads */
    ok("5-byte form",      (const unsigned char *)"\xF8\x88\x80\x80\x80", 5, -1, 0);
    ok("lead FE",          (const unsigned char *)"\xFE\x80\x80\x80", 4, -1, 0);
    ok("lead FF",          (const unsigned char *)"\xFF\x80\x80\x80", 4, -1, 0);
    /* structural errors */
    ok("continuation 1st", (const unsigned char *)"\x80\x41", 2, -1, 0);
    ok("bad continuation", (const unsigned char *)"\xE2\x41\xAC", 3, -1, 0);
    ok("truncated 2b",     (const unsigned char *)"\xC2", 1, -1, 0);
    ok("truncated 3b",     (const unsigned char *)"\xE2\x82", 2, -1, 0);
    ok("truncated 4b",     (const unsigned char *)"\xF0\x9F\x98", 3, -1, 0);
    ok("empty",            (const unsigned char *)"", 0, -1, 0);

    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}
""",
    },
    {
        "name": "csv_field",
        "sig": "int csv_field(const char *line, int idx, char *out, size_t cap)",
        "prompt": ("Extracts field idx (0-based) from one RFC-4180 CSV line into out "
                   "(NUL-terminated, truncated to cap-1 chars). Quoted fields may contain "
                   "commas and doubled quotes '\"\"' which unescape to a single quote. "
                   "Returns the field's unescaped length, or -1 if idx is out of range."),
        "hard": "1",
        "test": r"""
#include <stdio.h>
#include <string.h>
#include <stddef.h>
int csv_field(const char *line, int idx, char *out, size_t cap);
static int fails = 0;
static void ok(const char *line, int idx, const char *want, int wantlen) {
    char buf[128]; memset(buf, 0, sizeof buf);
    int n = csv_field(line, idx, buf, sizeof buf);
    if (n != wantlen || strcmp(buf, want)) {
        printf("FAIL csv_field(\"%s\", %d) = \"%s\" (%d), want \"%s\" (%d)\n",
               line, idx, buf, n, want, wantlen); fails++;
    }
}
int main(void) {
    ok("a,b,c", 0, "a", 1); ok("a,b,c", 2, "c", 1); ok("a,,c", 1, "", 0);
    ok("\"a,b\",c", 0, "a,b", 3); ok("\"a,b\",c", 1, "c", 1);
    ok("\"say \"\"hi\"\"\",x", 0, "say \"hi\"", 8);
    ok("x,\"say \"\"hi\"\"\"", 1, "say \"hi\"", 8);
    ok("one,two", 5, "", -1); ok("", 0, "", 0);
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
        # tolerate a trailing comment after the header: gemma writes
        # "include <stddef.h> // for size_t", which the old pattern missed
        m = re.match(r"^(?:[a-z]*\s*)?(?:import|include)\s*(<[^>]+>)", s)
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


def grade(task: dict[str, str], code: str, workdir: Path,
          note_limit: int = 100) -> tuple[str, str]:
    if not code:
        return "no_code", ""
    sol = workdir / "solution.c"
    sol.write_text(code)
    (workdir / "test.c").write_text(task["test"])
    # -Dmain=... neutralizes any stray main() in the solution.
    compile_sol = subprocess.run(
        ["cc", "-std=c11", "-O1", "-Wall", "-Dmain=solution_unused_main",
         "-c", "solution.c", "-o", "solution.o"],
        cwd=workdir, capture_output=True, text=True, errors="replace", timeout=60,
    )
    if compile_sol.returncode != 0:
        return "compile_error", compile_sol.stderr.strip()[:note_limit] if compile_sol.stderr else ""
    link = subprocess.run(
        ["cc", "test.c", "solution.o", "-o", "test_bin"],
        cwd=workdir, capture_output=True, text=True, errors="replace", timeout=60,
    )
    if link.returncode != 0:
        return "link_error", link.stderr.strip()[-note_limit:] if link.stderr else ""
    try:
        run = subprocess.run(
            ["./test_bin"], cwd=workdir, capture_output=True, text=True, errors="replace", timeout=RUN_TIMEOUT,
        )
    except subprocess.TimeoutExpired:
        return "timeout", ""
    if run.returncode == 0 and "PASS" in run.stdout:
        return "pass", ""
    first_fail = next((l for l in run.stdout.splitlines() if l.startswith("FAIL")), "")
    return "wrong_answer", first_fail[:note_limit]


def task_set(which: str) -> list[dict[str, str]]:
    # Brutal tasks are opt-in only: they stay out of easy/hard/all so scores
    # from earlier runs remain comparable.
    if which == "brutal":
        return [t for t in TASKS if t.get("brutal")]
    pool = [t for t in TASKS if not t.get("brutal")]
    if which == "easy":
        return [t for t in pool if not t.get("hard")]
    if which == "hard":
        return [t for t in pool if t.get("hard")]
    return pool


def eval_target(name: str, timeout: float, trials: int, dump_dir: str | None = None,
                tasks: list[dict[str, str]] | None = None) -> dict[str, Any]:
    cfg = TARGETS[name]
    tasks = tasks if tasks is not None else TASKS
    results: dict[str, list[str]] = {}
    notes: dict[str, str] = {}
    times: dict[str, list[float]] = {}
    tokens: dict[str, list[int]] = {}
    for task in tasks:
        prompt = build_prompt(task)
        outcomes: list[str] = []
        max_tok = MAX_TOKENS_HARMONY if name in HARMONY_TARGETS else MAX_TOKENS
        if name in THINKING_TARGETS:
            max_tok = MAX_TOKENS_HARMONY
        if task.get("brutal"):
            # brutal tasks need room to reason; the normal caps truncate mid-answer
            # and we'd be scoring the cut-off, not the model
            max_tok = MAX_TOKENS_BRUTAL_HARMONY if max_tok > MAX_TOKENS else MAX_TOKENS_BRUTAL
        for trial in range(trials):
            temp = 0.0 if trial == 0 else 0.7
            try:
                resp = complete_openai_full(
                    port=cfg["port"], model=cfg["model"], prompt=prompt,
                    max_tokens=max_tok, timeout=timeout, temperature=temp,
                )
                reply = resp["text"]
                times.setdefault(task["name"], []).append(round(resp["elapsed_s"], 2))
                if resp["completion_tokens"]:
                    tokens.setdefault(task["name"], []).append(resp["completion_tokens"])
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
            if status != "pass" and resp.get("completion_tokens", 0) >= max_tok:
                # ran out of budget mid-answer; report that rather than the
                # syntax error the truncation happens to produce
                status, note = "truncated", f"hit the {max_tok}-token cap"
            if status != "pass" and dump_dir and code:
                safe = "".join(c if c.isalnum() else "-" for c in name)
                Path(dump_dir).mkdir(parents=True, exist_ok=True)
                (Path(dump_dir) / f"{safe}-{task['name']}-t{trial}.c").write_text(code)
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
        "total": len(tasks) * trials,
        "trials": trials,
        "results": results,
        "notes": notes,
        "time_s": times,
        "tokens": tokens,
        "total_time_s": round(sum(sum(v) for v in times.values()), 1),
        "total_tokens": sum(sum(v) for v in tokens.values()),
    }


def print_table(rows: list[dict[str, Any]]) -> None:
    names = [r["target"] for r in rows]
    print()
    header = f"{'task':<16}" + "".join(f"{n:>10}" for n in names)
    print(header)
    print("-" * len(header))
    for task in TASKS:
        if not any(task["name"] in r["results"] for r in rows):
            continue
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
    parser.add_argument("--dump-failures", metavar="DIR", default=None,
                        help="save extracted code of failing attempts to DIR")
    parser.add_argument("--set", choices=("easy", "hard", "all", "brutal"), default="all",
                        dest="task_set", help="easy = original tasks, hard = harder tasks only")
    args = parser.parse_args()
    tasks = task_set(args.task_set)
    if args.target == "all":
        names = [n for n, c in TARGETS.items() if c["kind"] == "openai"]
    else:
        names = ["mlx", "ornith"] if args.target == "both" else [args.target]
    rows = [eval_target(name, args.timeout, args.trials, args.dump_failures, tasks) for name in names]
    if args.json:
        print(json.dumps(rows, indent=2))
    else:
        print_table(rows)


if __name__ == "__main__":
    main()
