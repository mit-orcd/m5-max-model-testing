def match(pattern, path):
    # Tokenize pattern into segments
    segs = []
    i = 0
    n = len(pattern)
    while i < n:
        if pattern[i] == '/':
            segs.append('/')
            i += 1
        elif pattern[i] == '\\' and i + 1 < n:
            segs.append(pattern[i + 1])
            i += 2
        else:
            buf = []
            while i < n and pattern[i] != '/' and pattern[i] != '\\':
                buf.append(pattern[i])
                i += 1
            segs.append(''.join(buf))
    # Split into path segments
    parts = []
    cur = []
    for t in segs:
        if t == '/':
            parts.append(cur)
            cur = []
        else:
            cur.append(t)
    parts.append(cur)
    # Now parts are lists of tokens per segment; '**' is a special token
    pat = parts
    txt = path.split('/')
    m = len(pat)
    k = len(txt)
    # dp[i][j] = pattern parts[i:] matches text segments txt[j:]
    from functools import lru_cache
    @lru_cache(maxsize=None)
    def dp(i, j):
        if i == m:
            return j == k
        if i < m and len(pat[i]) == 1 and pat[i][0] == '**':
            # '**' matches zero or more whole segments
            # try matching zero segments
            if dp(i + 1, j):
                return True
            # try consuming one segment and staying
            if j < k and dp(i, j + 1):
                return True
            return False
        # normal segment
        if j < k and seg_match(pat[i], txt[j]):
            return dp(i + 1, j + 1)
        return False
    def seg_match(pat_tokens, text):
        # token list -> regex matcher
        return seg_match_rec(pat_tokens, text, 0, 0)
    def seg_match_rec(tok, text, ti, xi):
        if ti == len(tok):
            return xi == len(text)
        t = tok[ti]
        if t == '*':
            # zero or more non-'/' chars
            # try all lengths
            for l in range(xi, len(text) + 1):
                if seg_match_rec(tok, text, ti + 1, l):
                    return True
            return False
        elif t == '?':
            if xi < len(text):
                return seg_match_rec(tok, text, ti + 1, xi + 1)
            return False
        elif t.startswith('['):
            # character class token (may include ranges and negation)
            if xi < len(text) and class_match(t, text[xi]):
                return seg_match_rec(tok, text, ti + 1, xi + 1)
            return False
        else:
            # literal char
            if xi < len(text) and t == text[xi]:
                return seg_match_rec(tok, text, ti + 1, xi + 1)
            return False
    def class_match(tok, ch):
        # tok like [abc], [a-z], [!...]
        neg = False
        inner = tok[1:]
        if inner[0] == '!':
            neg = True
            inner = inner[1:]
        # inner ends with ']'
        inner = inner[:-1]
        # parse ranges and chars
        chars = set()
        i = 0
        while i < len(inner):
            c = inner[i]
            if i + 2 < len(inner) and inner[i+1] == '-' and inner[i+2] != ']':
                a = inner[i]
                b = inner[i+2]
                for cc in range(ord(a), ord(b) + 1):
                    chars.add(chr(cc))
                i += 3
            else:
                chars.add(c)
                i += 1
        return (ch in chars) != neg
    return dp(0, 0)