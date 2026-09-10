#!/usr/bin/env python3
"""Sanity-checks the brutal task set before any model sees it.

Each task gets two solutions run through the real grader: a correct reference
that must pass, and a plausible naive attempt that must fail. A task that the
reference can't pass is broken; a task the naive version passes isn't brutal.
"""
from __future__ import annotations

import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import eval_bash
import eval_code
import eval_python

PY = sys.executable

C_GOOD = r"""
#include <stdint.h>
#include <stddef.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    unsigned char b0 = s[0];
    int len;
    uint32_t v, min;
    if (b0 < 0x80) { *cp = b0; return 1; }
    else if ((b0 & 0xE0) == 0xC0) { len = 2; v = b0 & 0x1Fu; min = 0x80; }
    else if ((b0 & 0xF0) == 0xE0) { len = 3; v = b0 & 0x0Fu; min = 0x800; }
    else if ((b0 & 0xF8) == 0xF0) { len = 4; v = b0 & 0x07u; min = 0x10000; }
    else return -1;
    if (n < (size_t)len) return -1;
    for (int i = 1; i < len; i++) {
        if ((s[i] & 0xC0) != 0x80) return -1;
        v = (v << 6) | (uint32_t)(s[i] & 0x3Fu);
    }
    if (v < min) return -1;
    if (v > 0x10FFFF) return -1;
    if (v >= 0xD800 && v <= 0xDFFF) return -1;
    *cp = v;
    return len;
}
"""

# decodes correctly but skips the overlong / surrogate / range checks
C_NAIVE = r"""
#include <stdint.h>
#include <stddef.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    unsigned char b0 = s[0];
    int len;
    uint32_t v;
    if (b0 < 0x80) { *cp = b0; return 1; }
    else if ((b0 & 0xE0) == 0xC0) { len = 2; v = b0 & 0x1Fu; }
    else if ((b0 & 0xF0) == 0xE0) { len = 3; v = b0 & 0x0Fu; }
    else if ((b0 & 0xF8) == 0xF0) { len = 4; v = b0 & 0x07u; }
    else return -1;
    if (n < (size_t)len) return -1;
    for (int i = 1; i < len; i++) {
        if ((s[i] & 0xC0) != 0x80) return -1;
        v = (v << 6) | (uint32_t)(s[i] & 0x3Fu);
    }
    *cp = v;
    return len;
}
"""

PY_GOOD = '''
def clone(obj, _memo=None, _keep=None):
    if _memo is None:
        _memo, _keep = {}, []
    key = id(obj)
    if key in _memo:
        return _memo[key]
    if isinstance(obj, dict):
        out = {}
        _memo[key] = out
        _keep.append(obj)
        for k, v in obj.items():
            out[clone(k, _memo, _keep)] = clone(v, _memo, _keep)
        return out
    if isinstance(obj, list):
        out = []
        _memo[key] = out
        _keep.append(obj)
        for v in obj:
            out.append(clone(v, _memo, _keep))
        return out
    if isinstance(obj, tuple):
        out = tuple(clone(v, _memo, _keep) for v in obj)
        _memo[key] = out
        _keep.append(obj)
        return out
    return obj
'''

# handles cycles with a visited set, so shared references get duplicated
PY_NAIVE = '''
def clone(obj, _seen=None):
    if _seen is None:
        _seen = set()
    if id(obj) in _seen:
        return obj
    if isinstance(obj, dict):
        _seen.add(id(obj))
        return {k: clone(v, _seen) for k, v in obj.items()}
    if isinstance(obj, list):
        _seen.add(id(obj))
        return [clone(v, _seen) for v in obj]
    if isinstance(obj, tuple):
        _seen.add(id(obj))
        return tuple(clone(v, _seen) for v in obj)
    return obj
'''

SH_GOOD = r"""
total_size() {
    local dir="$1" total=0 f sz
    while IFS= read -r -d '' f; do
        sz=$(wc -c < "$f")
        total=$(( total + sz ))
    done < <(find "$dir" -type f -print0)
    printf '%s\n' "$total"
}
"""

# word-splits on filenames and loses the total to the pipeline subshell
SH_NAIVE = r"""
total_size() {
    local total=0
    find "$1" -type f | while read f; do
        total=$((total + $(stat -f%z $f)))
    done
    echo $total
}
"""

ARENA_GOOD = r"""
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct Block { size_t size; size_t prev; size_t free; } Block;

#define A (_Alignof(max_align_t))
#define HDRSZ ((((sizeof(Block)) + A - 1) / A) * A)

static unsigned char *ar_base;
static unsigned char *ar_end;

static size_t roundup(size_t n) { return (n + A - 1) / A * A; }

void arena_init(void *buf, size_t size) {
    uintptr_t raw = (uintptr_t)buf;
    uintptr_t al = (raw + A - 1) / A * A;
    size_t off = (size_t)(al - raw);
    ar_base = ar_end = NULL;
    if (size < off + HDRSZ + A) return;
    ar_base = (unsigned char *)al;
    size_t usable = (size - off) / A * A;
    ar_end = ar_base + usable;
    Block *b = (Block *)ar_base;
    b->size = usable - HDRSZ;
    b->prev = 0;
    b->free = 1;
}

static Block *nextb(Block *b) {
    unsigned char *p = (unsigned char *)b + HDRSZ + b->size;
    return (p < ar_end) ? (Block *)p : NULL;
}
static Block *prevb(Block *b) {
    if (!b->prev) return NULL;
    return (Block *)((unsigned char *)b - b->prev - HDRSZ);
}
static void split(Block *b, size_t need) {
    if (b->size >= need + HDRSZ + A) {
        Block *n = (Block *)((unsigned char *)b + HDRSZ + need);
        n->size = b->size - need - HDRSZ;
        n->prev = need;
        n->free = 1;
        b->size = need;
        Block *nn = nextb(n);
        if (nn) nn->prev = n->size;
    }
}
static void merge_next(Block *b) {
    Block *n = nextb(b);
    if (n && n->free) {
        b->size += HDRSZ + n->size;
        Block *nn = nextb(b);
        if (nn) nn->prev = b->size;
    }
}
void *arena_alloc(size_t n) {
    if (!ar_base || n == 0) return NULL;
    size_t need = roundup(n);
    for (Block *b = (Block *)ar_base; b; b = nextb(b)) {
        if (b->free && b->size >= need) {
            split(b, need);
            b->free = 0;
            return (unsigned char *)b + HDRSZ;
        }
    }
    return NULL;
}
void arena_free(void *p) {
    if (!p || !ar_base) return;
    Block *b = (Block *)((unsigned char *)p - HDRSZ);
    b->free = 1;
    merge_next(b);
    Block *pv = prevb(b);
    if (pv && pv->free) merge_next(pv);
}
void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    Block *b = (Block *)((unsigned char *)p - HDRSZ);
    size_t need = roundup(n);
    if (b->size >= need) {
        split(b, need);
        Block *t = nextb(b);
        if (t && t->free) merge_next(t);
        return p;
    }
    Block *nx = nextb(b);
    if (nx && nx->free && b->size + HDRSZ + nx->size >= need) {
        b->size += HDRSZ + nx->size;
        Block *nn = nextb(b);
        if (nn) nn->prev = b->size;
        split(b, need);
        return p;
    }
    void *q = arena_alloc(n);
    if (!q) return NULL;
    memcpy(q, p, b->size < n ? b->size : n);
    arena_free(p);
    return q;
}
"""

# first-fit free list with no coalescing and a realloc that always moves
ARENA_NAIVE = r"""
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct Block { size_t size; int free; struct Block *next; } Block;
#define A (_Alignof(max_align_t))
#define HDRSZ ((((sizeof(Block)) + A - 1) / A) * A)

static unsigned char *base, *end, *bump;
static Block *head;

void arena_init(void *buf, size_t size) {
    uintptr_t al = ((uintptr_t)buf + A - 1) / A * A;
    base = bump = (unsigned char *)al;
    end = (unsigned char *)buf + size;
    head = NULL;
}
void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t need = (n + A - 1) / A * A;
    for (Block *b = head; b; b = b->next)
        if (b->free && b->size >= need) { b->free = 0; return (unsigned char *)b + HDRSZ; }
    if (bump + HDRSZ + need > end) return NULL;
    Block *b = (Block *)bump;
    bump += HDRSZ + need;
    b->size = need; b->free = 0; b->next = head; head = b;
    return (unsigned char *)b + HDRSZ;
}
void arena_free(void *p) {
    if (!p) return;
    ((Block *)((unsigned char *)p - HDRSZ))->free = 1;
}
void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    Block *b = (Block *)((unsigned char *)p - HDRSZ);
    if (b->size >= n) return p;
    void *q = arena_alloc(n);
    if (!q) return NULL;
    memcpy(q, p, b->size);
    arena_free(p);
    return q;
}
"""

CSV_GOOD = r"""
csv_to_tsv() {
    local file=$1 data n i ch nx
    local field row inq first
    data=$(cat -- "$file"; printf X); data=${data%X}
    n=${#data}; i=0; inq=0; field=''; row=''; first=1

    _ff() { if (( first )); then row=$field; first=0; else row+=$'\t'$field; fi; field=''; }
    _fr() { _ff; printf '%s\n' "$row"; row=''; first=1; }

    while (( i < n )); do
        ch=${data:i:1}
        if (( inq )); then
            case $ch in
                '"') nx=${data:i+1:1}
                     if [[ $nx == '"' ]]; then field+='"'; i=$((i+2)); else inq=0; i=$((i+1)); fi ;;
                $'\r') if [[ ${data:i+1:1} == $'\n' ]]; then i=$((i+2)); else i=$((i+1)); fi
                       field+='\n' ;;
                $'\n') field+='\n'; i=$((i+1)) ;;
                $'\t') field+='\t'; i=$((i+1)) ;;
                *) field+=$ch; i=$((i+1)) ;;
            esac
            continue
        fi
        case $ch in
            '"') inq=1; i=$((i+1)) ;;
            ',') _ff; i=$((i+1)) ;;
            $'\r') if [[ ${data:i+1:1} == $'\n' ]]; then i=$((i+2)); else i=$((i+1)); fi; _fr ;;
            $'\n') _fr; i=$((i+1)) ;;
            $'\t') field+='\t'; i=$((i+1)) ;;
            *) field+=$ch; i=$((i+1)) ;;
        esac
    done
    if (( n > 0 )) && [[ ${data: -1} != $'\n' ]]; then _fr; fi
}
"""

# splits on every comma and never looks at quotes
CSV_NAIVE = r"""
csv_to_tsv() {
    while IFS= read -r line; do
        line=${line%$'\r'}
        printf '%s\n' "${line//,/$'\t'}"
    done < "$1"
}
"""

GLOB_GOOD = r'''
def _inclass(body, ch):
    k = 0
    while k < len(body):
        if k + 2 < len(body) and body[k + 1] == "-":
            if body[k] <= ch <= body[k + 2]:
                return True
            k += 3
        else:
            if body[k] == ch:
                return True
            k += 1
    return False


def _seg(p, i, s, j):
    while i < len(p):
        c = p[i]
        if c == "*":
            for k in range(j, len(s) + 1):
                if _seg(p, i + 1, s, k):
                    return True
            return False
        if j >= len(s):
            return False
        if c == "?":
            i += 1; j += 1; continue
        if c == "\\":
            i += 1
            if i >= len(p) or p[i] != s[j]:
                return False
            i += 1; j += 1; continue
        if c == "[":
            end = i + 1
            neg = False
            if end < len(p) and p[end] == "!":
                neg = True; end += 1
            start = end
            while end < len(p) and (end == start or p[end] != "]"):
                end += 1
            if end >= len(p):
                if s[j] != "[":
                    return False
                i += 1; j += 1; continue
            if _inclass(p[start:end], s[j]) != neg:
                i = end + 1; j += 1; continue
            return False
        if c != s[j]:
            return False
        i += 1; j += 1
    return j == len(s)


def _segs(p, i, s, j):
    while i < len(p):
        if p[i] == "**":
            for k in range(j, len(s) + 1):
                if _segs(p, i + 1, s, k):
                    return True
            return False
        if j >= len(s) or not _seg(p[i], 0, s[j], 0):
            return False
        i += 1; j += 1
    return j == len(s)


def match(pattern, path):
    return _segs(pattern.split("/"), 0, path.split("/"), 0)
'''

# regex translation: * crosses separators and ** cannot match zero segments
GLOB_NAIVE = r'''
import re


def match(pattern, path):
    rx = re.escape(pattern).replace(r"\*\*", ".*").replace(r"\*", "[^/]*").replace(r"\?", ".")
    return re.fullmatch(rx, path) is not None
'''

CASES = [
    ("C", "arena_alloc", eval_code, ARENA_GOOD, ARENA_NAIVE),
    ("C", "utf8_next", eval_code, C_GOOD, C_NAIVE),
    ("Python", "path_glob", eval_python, GLOB_GOOD, GLOB_NAIVE),
    ("Python", "clone_graph", eval_python, PY_GOOD, PY_NAIVE),
    ("Bash", "csv_to_tsv", eval_bash, CSV_GOOD, CSV_NAIVE),
    ("Bash", "total_size", eval_bash, SH_GOOD, SH_NAIVE),
]


def run(mod, task: dict, code: str) -> tuple[str, str]:
    with tempfile.TemporaryDirectory() as td:
        wd = Path(td)
        if mod is eval_python:
            return mod.grade(task, code, wd, PY, note_limit=400)
        if mod is eval_bash:
            return mod.grade(task, code, wd, note_limit=400)
        return mod.grade(task, code, wd, note_limit=400)


def main() -> int:
    bad = 0
    for lang, name, mod, good, naive in CASES:
        tasks = {t["name"]: t for t in mod.task_set("brutal")}
        if name not in tasks:
            print(f"{lang:7} {name}: MISSING from the brutal set")
            bad += 1
            continue
        task = tasks[name]

        status, note = run(mod, task, good)
        if status == "pass":
            print(f"{lang:7} {name}: reference passes")
        else:
            print(f"{lang:7} {name}: BROKEN - reference got {status}: {note}")
            bad += 1

        status, note = run(mod, task, naive)
        if status != "pass":
            first = (note or "").strip().splitlines()
            print(f"{lang:7} {name}: naive fails ({status}) "
                  f"-> {first[0] if first else ''}")
        else:
            print(f"{lang:7} {name}: TOO EASY - the naive solution passes")
            bad += 1

    seen = set()
    for lang, _, mod, _, _ in CASES:
        if lang in seen:
            continue
        seen.add(lang)
        counts = {w: len(mod.task_set(w)) for w in ("easy", "hard", "all", "brutal")}
        print(f"{lang:7} set sizes: {counts}")
    return 1 if bad else 0


if __name__ == "__main__":
    raise SystemExit(main())
