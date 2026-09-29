def match(pattern, path):
    p = pattern.split('/')
    s = path.split('/')
    return _match(p, s, len(p), len(s))

def _match(p, s, i, j):
    while i < len(p):
        if p[i] == '**':
            # '**' matches zero or more segments
            for k in range(j, len(s) + 1):
                if _match(p, s, i + 1, k):
                    return True
            return False
        if j >= len(s):
            return False
        if not _segment_match(p[i], s[j]):
            return False
        i += 1
        j += 1
    return j == len(s)

def _segment_match(pat, seg):
    # returns True if pattern segment matches exactly the segment
    return _seg_match(pat, seg, 0, 0, len(pat), len(seg))

def _seg_match(pat, seg, i, j, pi, si):
    while i < pi:
        c = pat[i]
        if c == '\\':
            if i + 1 >= pi:
                return False
            if j >= si or seg[j] != pat[i + 1]:
                return False
            i += 2
            j += 1
        elif c == '[':
            if j >= si:
                return False
            cls = _parse_class(pat, i)
            if cls is None:
                return False
            if not cls[0](seg[j]):
                return False
            i = cls[1]
            j += 1
        elif c == '*':
            # '*' matches zero or more non-'/' chars
            # try to match greedily, backtracking
            # find remaining pattern after '*' and match it
            # We need to match rest of pattern against suffix
            rest = pat[i + 1:pi]
            # try all split points
            for k in range(j, si + 1):
                if _seg_match(pat, seg, i + 1, k, pi, si):
                    return True
            return False
        elif c == '?':
            if j >= si:
                return False
            j += 1
            i += 1
        else:
            if j >= si or seg[j] != c:
                return False
            i += 1
            j += 1
    return j == si

def _parse_class(pat, i):
    # pat[i] == '['
    # returns (match_func, next_index_after_closing_bracket)
    n = len(pat)
    # handle '!' or '^' as negation
    neg = False
    k = i + 1
    if k < n and pat[k] in '!^':
        neg = True
        k += 1
    chars = []  # list of (set, is_range)
    while k < n:
        if pat[k] == ']':
            k += 1
            break
        if pat[k] == '\\':
            # escaped char inside class
            if k + 1 >= n:
                return None
            chars.append(({pat[k + 1]}, False))
            k += 2
        else:
            if k + 2 < n and pat[k + 1] == '-' and pat[k + 2] != ']':
                # range
                lo = pat[k]
                hi = pat[k + 2]
                if lo <= hi:
                    chars.append((set(chr(c) for c in range(ord(lo), ord(hi) + 1)), True))
                else:
                    chars.append(({lo, hi}, True))
                k += 3
            else:
                chars.append(({pat[k]}, False))
                k += 1
    if k > n:
        return None
    # build match function
    def matcher(ch):
        for char_set, _ in chars:
            if ch in char_set:
                return not neg
        return neg
    return matcher, k