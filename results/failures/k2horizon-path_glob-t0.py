def match(pattern, path):
    def parse_class(s, i):
        # s[i] == '['
        j = i + 1
        neg = False
        if j < len(s) and s[j] == '!':
            neg = True
            j += 1
        chars = set()
        ranges = []
        while j < len(s) and s[j] != ']':
            if j + 2 < len(s) and s[j+1] == '-' and s[j+2] != ']':
                lo, hi = s[j], s[j+2]
                if lo <= hi:
                    for c in range(ord(lo), ord(hi)+1):
                        chars.add(chr(c))
                j += 3
            else:
                chars.add(s[j])
                j += 1
        if j >= len(s):
            # unterminated class: treat '[' literally
            return None, i + 1
        j += 1  # skip ']'
        return (neg, chars), j

    def parse_token(s, i):
        # returns (kind, value, next_i)
        # kind: 'lit', 'any', 'star', 'dstar', 'class'
        if i >= len(s):
            return None, i
        c = s[i]
        if c == '*':
            if i + 1 < len(s) and s[i+1] == '*':
                return 'dstar', None, i + 2
            return 'star', None, i + 1
        if c == '?':
            return 'any', None, i + 1
        if c == '[':
            res = parse_class(s, i)
            if res[0] is not None:
                return 'class', res[0], res[1]
            return 'lit', '[', i + 1
        if c == '\\':
            if i + 1 < len(s):
                return 'lit', s[i+1], i + 2
            return 'lit', '\\', i + 1
        return 'lit', c, i + 1

    def tokenize(p):
        toks = []
        i = 0
        while i < len(p):
            kind, val, ni = parse_token(p, i)
            if kind is None:
                break
            toks.append((kind, val))
            i = ni
        return toks

    def seg_match(tok, seg):
        kind, val = tok
        if kind == 'lit':
            return seg == val
        if kind == 'any':
            return len(seg) == 1
        if kind == 'star':
            return True  # '*' matches any segment (zero or more chars, never '/')
        if kind == 'class':
            if '/' in seg or len(seg) != 1:
                return False
            neg, chars = val
            return (seg[0] in chars) != neg
        return False

    def dp_match(toks, segs):
        # dp[i][j]: toks[i:] matches segs[j:]
        n, m = len(toks), len(segs)
        memo = {}
        def rec(i, j):
            if (i, j) in memo:
                return memo[(i, j)]
            if i == n:
                res = (j == m)
            elif toks[i][0] == 'dstar':
                # zero segments
                res = rec(i + 1, j)
                if not res:
                    for k in range(j, m):
                        if rec(i, k + 1):
                            res = True
                            break
            else:
                if j < m and seg_match(toks[i], segs[j]):
                    res = rec(i + 1, j + 1)
                else:
                    res = False
            memo[(i, j)] = res
            return res
        return rec(0, 0)

    toks = tokenize(pattern)
    segs = path.split('/')
    return dp_match(toks, segs)