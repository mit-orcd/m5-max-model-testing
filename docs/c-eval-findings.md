# C Eval Findings

16 C11 tasks × 3 trials (temp 0, then 0.7), each compiled with `cc -std=c11 -Wall` and run against a hidden test harness. No LLM judge — code either compiles and passes, or it doesn't. All failing samples are preserved in [`results/failures/`](../results/failures/).

## Per-task matrix (passes / 3 trials)

| task | qwen27 | ornith | coder | qwen35 | gptoss | gemma | devstral | aya | qwen36-27b | qwen36-35b | glm-flash | coder-next | deepseek |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| reverse_string | 3 | 1 | 2 | 3 | 2 | 3 | 2 | 0 | 0 | 2 | 0 | 3 | 1 |
| is_prime | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 0 | 3 | 3 |
| fizzbuzz | 3 | 0 | 0 | 1 | 3 | 3 | 2 | 2 | 3 | 2 | 1 | 3 | 0 |
| binary_search | 3 | 3 | 0 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 2 | 3 | 3 |
| count_words | 3 | 3 | 2 | 0 | 3 | 2 | 3 | 1 | 0 | 0 | 0 | 3 | 2 |
| max_subarray | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 1 | 3 | 3 | 3 | 3 | 3 |
| itoa | 1 | 2 | 0 | 1 | 3 | 3 | 3 | 1 | 1 | 2 | 0 | 3 | 0 |
| stack | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 2 | 3 | 3 |
| popcount | 0 | 3 | 0 | 3 | 3 | 3 | 3 | 2 | 3 | 3 | 3 | 3 | 3 |
| rot13 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 0 | 3 | 3 | 2 | 3 | 3 |
| parse_csv_ints | 3 | 2 | 0 | 1 | 3 | 1 | 3 | 0 | 3 | 0 | 0 | 3 | 0 |
| bswap32 | 3 | 3 | 3 | 3 | 2 | 3 | 3 | 2 | 3 | 3 | 2 | 3 | 0 |
| trim | 0 | 0 | 0 | 2 | 0 | 3 | 1 | 0 | 0 | 1 | 0 | 0 | 0 |
| cmp_desc | 3 | 1 | 3 | 0 | 3 | 3 | 2 | 0 | 1 | 3 | 0 | 3 | 1 |
| ring | 3 | 0 | 3 | 3 | 3 | 3 | 3 | 0 | 3 | 3 | 2 | 3 | 1 |
| atoi_strict | 3 | 0 | 0 | 0 | 3 | 3 | 2 | 0 | 0 | 0 | 0 | 2 | 0 |
| **total /48** | **40** | **30** | **25** | **32** | **43** | **45** | **42** | **18** | **32** | **34** | **17** | **44** | **23** |

## Failure modes, with real examples

### 1. Missing includes (ornith, aya, coder)

The most common systematic failure: correct logic that doesn't compile because a header is missing. Ornith's `reverse_string` uses `NULL` with no header at all:

```c
void reverse_string(char *s) {
    if (s == NULL) return;   // error: use of undeclared identifier 'NULL'
    char *end = s;
    while (*end != '\0') end++;
    ...
}
```

An agent loop that feeds compiler errors back would fix these in one iteration — but zero-shot, it's a fail.

### 2. Python-isms in C output (qwen3-coder-30b)

The coding-specialist model's signature failure: emitting Python import syntax mid-C-file. From `coder-fizzbuzz-t0.c`:

```c
const char *fizzbuzz(int n) {
    static char buffer[100];
    if (n % 1import <stdio.h>      // ← Python import glued into C
#include <stdlib.h>
```

### 3. Token-level syntax garbage (glm-flash)

GLM-Flash was the worst offender for malformed output. Its `is_prime` (0/3) looks plausible until:

```c
for (int i = 5; i * i <= n; i += 6) {
    if (n % i == 0 || n % (i + 2) == 02;   // ← "02;" — unbalanced, invalid
        return 0;
    }
}
```

### 4. Inverted logic (qwen3.5-35b cmp_desc, 0/3)

Asked for a *descending* qsort comparator, it returned the ascending one — compiles cleanly, fails every test:

```c
int cmp_desc(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    if (x > y) return 1;    // backwards: descending wants -1 here
    if (x < y) return -1;
    return 0;
}
```

### 5. Off-by-one on pointer position (qwen27 itoa)

qwen27's `itoa` handles `0`, negatives, and `INT_MIN` carefully — then copies digits starting from the terminator it just wrote, so the loop body never runs and every conversion returns `""`:

```c
*t = '\0';
/* Reverse tmp into buf */
while (*t != '\0') {   // t points AT the '\0' — loop never executes
    *p++ = *t++;
}
*p = '\0';             // buf = ""
```

Test output: `FAIL itoa(7)="" want "7"`.

### 6. Bleeding-edge C that the toolchain rejects (qwen27 popcount, 0/3)

qwen27 reached for C23's `stdbit.h`, which Apple clang doesn't ship:

```
solution.c:1:10: fatal error: 'stdbit.h' file not found
```

Correct API, wrong ecosystem — a real-world-relevant failure mode.

### 7. The hardest task: `trim` (only gemma passed all 3)

In-place leading+trailing whitespace strip defeated almost everyone. Typical bug (qwen27): only trailing whitespace removed — `trim("  hello  ")` → `"  hello"`. gemma went 3/3; gpt-oss, qwen27, ornith, coder, coder-next all went 0/3.

## Quality-probe caveats

The 6 exact-match probes (`bench.py --case quality`) punish reasoning models for harness reasons:

- **gpt-oss (3/6):** Harmony `<|channel|>analysis` thinking leaks into the non-streaming reply path. The C eval strips it; the probes don't. Its probe failures look like:

  ```text
  <|channel|>analysis<|message|>The user asks: "What is 17 * 24? ...
  ```

- **deepseek-r1 (0/6):** the 64-token probe cap is consumed entirely by `<think>` content, so the visible reply is empty.

Both are serving/stack configuration issues, not model incapability — but they *are* real integration hazards for an agent pipeline.

## Takeaways

1. **Compile-and-run verification discriminates where perplexity can't.** Best-perplexity model (qwen3.5, 3.38) scored 32/48; gemma scored 45/48 with a broken-looking perplexity number.
2. **Failure modes are systematic, not noise** — the same model fails the same task the same way across trials (ornith's missing includes, qwen35's inverted comparator).
3. **The fast MoE cluster's failures are exactly the agentic-killer kind**: ignoring explicit instructions ("include this exact typedef"), malformed output, wrong-polarity logic.
4. **gemma-4-26b and qwen3-coder-next earned their top scores the hard way** — fewest systematic failures, including on `trim` and `atoi_strict`, the two tasks almost everyone failed.
