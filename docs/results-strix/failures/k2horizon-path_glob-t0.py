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
                for c in range(ord(lo), ord(hi)+1):
                    chars.add(chr(c))
                j += 3
            else:
                chars.add(s[j])
                j += 1
        if j >= len(s):
            # no closing bracket; treat '[' literally
            return None, i + 1
        j += 1  # skip ']'
        return (neg, chars), j

    def parse_token(s, i):
        # returns (kind, value, next_i)
        if s[i] == '[':
            cls, ni = parse_class(s, i)
            if cls is not None:
                return ('class', cls, ni)
            return ('lit', '[', i+1)
        if s[i] == '\\':
            if i + 1 < len(s):
                return ('lit', s[i+1], i+2)
            return ('lit', '\\', i+1)
        if s[i] == '*':
            if i + 1 < len(s) and s[i+1] == '*':
                # '**' token; consume following '/' if present
                ni = i + 2
                if ni < len(s) and s[ni] == '/':
                    ni += 1
                return ('starstar', None, ni)
            return ('star', None, i+1)
        if s[i] == '?':
            return ('qmark', None, i+1)
        return ('lit', s[i], i+1)

    def tokenize(p):
        toks = []
        i = 0
        while i < len(p):
            kind, val, ni = parse_token(p, i)
            toks.append((kind, val))
            i = ni
        return toks

    def seg_match(toks, seg):
        # match a single path segment against tokens (no '/')
        n = len(toks)
        m = len(seg)
        # dp[i][j]: first i tokens match first j chars
        dp = [[False]*(m+1) for _ in range(n+1)]
        dp[0][0] = True
        for i in range(1, n+1):
            kind, val = toks[i-1]
            if kind == 'star':
                for j in range(m+1):
                    if dp[i-1][j]:
                        for k in range(j, m+1):
                            dp[i][k] = True
            elif kind == 'qmark':
                for j in range(m):
                    if dp[i-1][j]:
                        dp[i][j+1] = True
            elif kind == 'class':
                neg, chars = val
                for j in range(m):
                    if dp[i-1][j] and seg[j] not in chars:
                        if not neg:
                            dp[i][j+1] = True
                        else:
                            dp[i][j+1] = True
                    # handle negation properly below
                # redo negation correctly
                for j in range(m):
                    if dp[i-1][j]:
                        if neg:
                            if seg[j] not in chars:
                                dp[i][j+1] = True
                        else:
                            if seg[j] in chars:
                                dp[i][j+1] = True
            else:  # lit
                for j in range(m):
                    if dp[i-1][j] and seg[j] == val:
                        dp[i][j+1] = True
        return dp[n][m]

    def seg_match2(toks, seg):
        # cleaner implementation
        n = len(toks)
        m = len(seg)
        dp = [[False]*(m+1) for _ in range(n+1)]
        dp[0][0] = True
        for i in range(1, n+1):
            kind, val = toks[i-1]
            if kind == 'star':
                for j in range(m+1):
                    if dp[i-1][j]:
                        for k in range(j, m+1):
                            dp[i][k] = True
            elif kind == 'qmark':
                for j in range(m):
                    if dp[i-1][j]:
                        dp[i][j+1] = True
            elif kind == 'class':
                neg, chars = val
                for j in range(m):
                    if dp[i-1][j]:
                        if neg:
                            if seg[j] not in chars:
                                dp[i][j+1] = True
                        else:
                            if seg[j] in chars:
                                dp[i][j+1] = True
            else:
                for j in range(m):
                    if dp[i-1][j] and seg[j] == val:
                        dp[i][j+1] = True
        return dp[n][m]

    def full_match(toks, path):
        # split path into segments
        segs = path.split('/')
        # dp over pattern tokens and path segments
        # We process tokens; '**' can consume zero or more segments.
        # Use recursion with memo on (token_index, seg_index)
        from functools import lru_cache
        n = len(toks)
        S = len(segs)

        @lru_cache(maxsize=None)
        def rec(i, j):
            # tokens[i:] match segs[j:]
            if i == n:
                return j == S
            kind, val = toks[i]
            if kind == 'starstar':
                # zero segments
                if rec(i+1, j):
                    return True
                # one or more segments
                for k in range(j, S):
                    if rec(i, k+1):
                        return True
                return False
            else:
                # match one segment
                if j < S and seg_match2(toks[i:i+1], segs[j]):
                    return rec(i+1, j+1)
                return False

        return rec(0, 0)

    toks = tokenize(pattern)
    return full_match(toks, path)