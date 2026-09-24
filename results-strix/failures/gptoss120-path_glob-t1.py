import re
from functools import lru_cache

def _split_pattern(pat: str):
    """Split pattern on unescaped '/' keeping escape sequences."""
    segs, cur = [], []
    i = 0
    L = len(pat)
    while i < L:
        ch = pat[i]
        if ch == '\\':
            if i + 1 < L:
                cur.append(pat[i] + pat[i + 1])
                i += 2
                continue
            cur.append('\\')
            i += 1
            continue
        if ch == '/':
            segs.append(''.join(cur))
            cur = []
            i += 1
            continue
        cur.append(ch)
        i += 1
    segs.append(''.join(cur))
    return segs


def _segment_to_regex(seg: str):
    """Translate a single pattern segment (with escapes) to a regex pattern."""
    i, n = 0, len(seg)
    out = []
    while i < n:
        if seg[i] == '\\':                     # escaped literal
            if i + 1 < n:
                lit = seg[i + 1]
                out.append(re.escape(lit))
                i += 2
            else:
                out.append('\\\\')
                i += 1
            continue

        c = seg[i]
        if c == '*':
            out.append('.*')
        elif c == '?':
            out.append('.')
        elif c == '[':                         # character class
            j = i + 1
            neg = False
            if j < n and seg[j] == '!':
                neg = True
                j += 1
            chars = []
            while j < n:
                if seg[j] == '\\':               # escaped inside class
                    if j + 1 < n:
                        chars.append(re.escape(seg[j + 1]))
                        j += 2
                        continue
                    chars.append('\\\\')
                    j += 1
                    continue
                if seg[j] == ']':
                    break
                # escape regex meta‑chars inside class
                if seg[j] in r'\-^]':
                    chars.append('\\' + seg[j])
                else:
                    chars.append(seg[j])
                j += 1
            if j >= n or seg[j] != ']':          # no closing ], treat '[' literally
                out.append('\\[')
                i += 1
                continue
            cls = ''.join(chars)
            out.append('[^' + cls + ']' if neg else '[' + cls + ']')
            i = j                                     # will be incremented below
        else:
            out.append(re.escape(c))
        i += 1
    return '^' + ''.join(out) + '$'


# cache compiled regexes for speed
_regex_cache = {}
def _compile_seg(seg: str):
    if seg not in _regex_cache:
        _regex_cache[seg] = re.compile(_segment_to_regex(seg))
    return _regex_cache[seg]


def match(pattern: str, path: str) -> bool:
    """Return True iff the glob *pattern* matches the whole *path*."""
    p_segs = _split_pattern(pattern)
    s_segs = path.split('/')                     # path never contains escaped '/'

    @lru_cache(maxsize=None)
    def dfs(pi: int, si: int) -> bool:
        if pi == len(p_segs) and si == len(s_segs):
            return True
        if pi == len(p_segs):
            return False

        pseg = p_segs[pi]

        if pseg == '**':                         # match zero or more whole segments
            # try all possible consumptions
            for k in range(si, len(s_segs) + 1):
                if dfs(pi + 1, k):
                    return True
            return False

        # ordinary segment: must match exactly one path segment
        if si >= len(s_segs):
            return False
        if _compile_seg(pseg).match(s_segs[si]):
            return dfs(pi + 1, si + 1)
        return False

    return dfs(0, 0)