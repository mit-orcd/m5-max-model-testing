def match(pattern, path):
    def seg_match(pat, s):
        # classic glob within one segment: ?, *, [...], backslash escapes
        def rec(pi, si):
            while pi < len(pat):
                c = pat[pi]
                if c == '*':
                    while pi + 1 < len(pat) and pat[pi + 1] == '*':
                        pi += 1
                    if pi + 1 == len(pat):
                        return True
                    for k in range(si, len(s) + 1):
                        if rec(pi + 1, k):
                            return True
                    return False
                if si >= len(s):
                    return False
                if c == '?':
                    pi += 1; si += 1
                    continue
                if c == '\\':
                    pi += 1
                    if pi >= len(pat):
                        return False
                    if s[si] != pat[pi]:
                        return False
                    pi += 1; si += 1
                    continue
                if c == '[':
                    j = pi + 1
                    neg = j < len(pat) and pat[j] == '!'
                    if neg:
                        j += 1
                    ok = False
                    first = True
                    while j < len(pat) and (pat[j] != ']' or first):
                        first = False
                        if j + 2 < len(pat) and pat[j + 1] == '-' and pat[j + 2] != ']':
                            if pat[j] <= s[si] <= pat[j + 2]:
                                ok = True
                            j += 3
                        else:
                            if pat[j] == s[si]:
                                ok = True
                            j += 1
                    if j >= len(pat):
                        return False  # unterminated class
                    if ok == neg:
                        return False
                    pi = j + 1; si += 1
                    continue
                if s[si] != c:
                    return False
                pi += 1; si += 1
            return si == len(s)
        return rec(0, 0)

    pats = pattern.split('/')
    segs = path.split('/')

    def rec(pi, si):
        if pi == len(pats):
            return si == len(segs)
        if pats[pi] == '**':
            for k in range(si, len(segs) + 1):
                if rec(pi + 1, k):
                    return True
            return False
        if si >= len(segs):
            return False
        return seg_match(pats[pi], segs[si]) and rec(pi + 1, si + 1)

    return rec(0, 0)
