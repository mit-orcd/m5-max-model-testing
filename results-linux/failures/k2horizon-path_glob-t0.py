def match(pattern, path):
    def parse_class(s, i):
        # s[i] == '['
        j = i + 1
        neg = False
        if j < len(s) and s[j] == '!':
            neg = True
            j += 1
        chars = set()
        while j < len(s) and s[j] != ']':
            if j + 2 < len(s) and s[j+1] == '-' and s[j+2] != ']':
                for c in range(ord(s[j]), ord(s[j+2]) + 1):
                    chars.add(chr(c))
                j += 3
            else:
                chars.add(s[j])
                j += 1
        if j >= len(s):
            # no closing bracket; treat '[' literally
            return None, i + 1
        return (chars, neg), j + 1

    def parse_token(s, i):
        # returns (kind, value, next_i)
        if s[i] == '[':
            r = parse_class(s, i)
            if r[0] is not None:
                return ('class', r[0]), r[1]
            return ('lit', '['), i + 1
        if s[i] == '\\' and i + 1 < len(s):
            return ('lit', s[i+1]), i + 2
        return ('lit', s[i]), i + 1

    def tokenize(pat):
        toks = []
        i = 0
        while i < len(pat):
            if pat[i] == '*':
                if i + 1 < len(pat) and pat[i+1] == '*':
                    # '**' segment: must be exactly '**' as a whole segment
                    # check boundaries
                    start = i
                    j = i + 2
                    if j < len(pat) and pat[j] == '*':
                        # '***' -> treat first '*' as star, then '**' later
                        toks.append(('star', None))
                        i += 1
                        continue
                    # check that it's a whole segment: preceded by start or '/', followed by end or '/'
                    if (start == 0 or pat[start-1] == '/') and (j == len(pat) or pat[j] == '/'):
                        toks.append(('dstar', None))
                        i = j
                        continue
                    else:
                        toks.append(('star', None))
                        i += 1
                        continue
                toks.append(('star', None))
                i += 1
            elif pat[i] == '?':
                toks.append(('qmark', None))
                i += 1
            elif pat[i] == '[':
                r = parse_class(pat, i)
                if r[0] is not None:
                    toks.append(('class', r[0]))
                    i = r[1]
                else:
                    toks.append(('lit', '['))
                    i += 1
            elif pat[i] == '\\' and i + 1 < len(pat):
                toks.append(('lit', pat[i+1]))
                i += 2
            else:
                toks.append(('lit', pat[i]))
                i += 1
        return toks

    def seg_match(toks, seg):
        # match tokens against a single segment (no '/')
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
                chars, neg = val
                for j in range(m):
                    if dp[i-1][j] and ((seg[j] in chars) != neg):
                        dp[i][j+1] = True
            else:  # lit
                for j in range(m):
                    if dp[i-1][j] and seg[j] == val:
                        dp[i][j+1] = True
        return dp[n][m]

    def dstar_match(toks, segs, si, ti):
        # match tokens from index ti against segments from si, where toks[ti] is 'dstar'
        # '**' matches zero or more whole segments
        if ti + 1 == len(toks):
            return si == len(segs)
        # try consuming k segments
        k = 0
        while si + k <= len(segs):
            if seg_match(toks[ti+1:], segs[si+k]):
                return True
            k += 1
        return False

    def match_tokens(toks, segs):
        n = len(toks)
        m = len(segs)
        # dp[i][j]: first i tokens match first j segments
        dp = [[False]*(m+1) for _ in range(n+1)]
        dp[0][0] = True
        for i in range(1, n+1):
            kind, val = toks[i-1]
            if kind == 'dstar':
                for j in range(m+1):
                    if dp[i-1][j]:
                        # consume k segments
                        for k in range(0, m-j+1):
                            dp[i][j+k] = True
            elif kind == 'star':
                # star matches within a segment; handled at segment level
                # For segment-level matching, we need to match token against segment
                # We'll handle by matching tokens against each segment
                pass
            else:
                for j in range(1, m+1):
                    if dp[i-1][j-1] and seg_match([toks[i-1]], segs[j-1]):
                        dp[i][j] = True
        # Now handle stars within segments: we need a different approach.
        # Instead, split pattern into segments and match each segment with its tokens.
        return dp

    # Better approach: split pattern into segments, then match segment by segment.
    # But '**' complicates. Let's do recursive matching.

    def rec(toks, ti, segs, si):
        if ti == len(toks):
            return si == len(segs)
        kind, val = toks[ti]
        if kind == 'dstar':
            # try consuming k segments
            k = 0
            while si + k <= len(segs):
                if rec(toks, ti+1, segs, si+k):
                    return True
                k += 1
            return False
        else:
            # match this token against segment si
            if si >= len(segs):
                return False
            # We need to match a sequence of tokens against one segment.
            # Collect consecutive non-dstar tokens until next dstar or end.
            # Actually, tokens are matched against segments. A 'star' token can match
            # within a segment. So we need to match a group of tokens against one segment.
            # Find the maximal group of tokens that are not dstar.
            j = ti
            while j < len(toks) and toks[j][0] != 'dstar':
                j += 1
            # tokens ti..j-1 must match segment si
            if not seg_match(toks[ti:j], segs[si]):
                return False
            return rec(toks, j, segs, si+1)

    toks = tokenize(pattern)
    segs = path.split('/')
    return rec(toks, 0, segs, 0)