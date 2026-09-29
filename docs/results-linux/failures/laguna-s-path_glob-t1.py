def match(pattern, path):
    def tokenize(s):
        tokens = []
        i = 0
        while i < len(s):
            c = s[i]
            if c == '\\':
                if i + 1 < len(s):
                    tokens.append(('L', s[i + 1]))
                    i += 2
                    continue
                else:
                    tokens.append(('L', '\\'))
                    i += 1
                    continue
            if c == '[':
                j = i + 1
                negate = False
                if j < len(s) and s[j] == '!':
                    negate = True
                    j += 1
                ranges = []
                while j < len(s) and s[j] != ']':
                    if s[j] == '\\' and j + 1 < len(s):
                        ranges.append((s[j + 1], s[j + 1]))
                        j += 2
                    else:
                        start = s[j]
                        end = start
                        j += 1
                        if j + 1 < len(s) and s[j] == '-' and s[j + 1] != ']':
                            end = s[j + 1]
                            j += 2
                        ranges.append((start, end))
                if j < len(s):
                    j += 1  # skip ']'
                tokens.append(('C', negate, ranges))
                i = j
            else:
                tokens.append(('L', c))
                i += 1
        return tokens

    def seg_match(seg_tokens, seg_str):
        m, n = len(seg_tokens), len(seg_str)
        dp = [[False]*(n+1) for _ in range(m+1)]
        dp[0][0] = True
        for i in range(1, m+1):
            t = seg_tokens[i-1]
            if t[0] == 'L':
                ch = t[1]
                for j in range(1, n+1):
                    if seg_str[j-1] == ch:
                        dp[i][j] = dp[i-1][j-1]
            elif t[0] == '?':
                for j in range(1, n+1):
                    dp[i][j] = dp[i-1][j-1]
            elif t[0] == '*':
                for j in range(n+1):
                    if dp[i-1][j]:
                        for k in range(j, n+1):
                            dp[i][k] = True
                            if k == n:
                                break
                    if dp[i][j]:
                        break
            elif t[0] == 'C':
                _, negate, ranges = t
                for j in range(1, n+1):
                    ch = seg_str[j-1]
                    matched = False
                    for (a, b) in ranges:
                        if a <= ch <= b:
                            matched = True
                            break
                    if (matched and not negate) or (not matched and negate):
                        dp[i][j] = dp[i-1][j-1]
        return dp[m][n]

    def split_path(p):
        return p.split('/')

    def split_pattern(pat):
        return pat.split('/')

    def match_segments(pat_segs, path_segs):
        m, n = len(pat_segs), len(path_segs)
        dp = [[False]*(n+1) for _ in range(m+1)]
        dp[0][0] = True
        for i in range(m+1):
            for j in range(n+1):
                if not dp[i][j]:
                    continue
                if i < m:
                    pseg = pat_segs[i]
                    if pseg == '**':
                        # match zero segments
                        dp[i+1][j] = True
                        # match one or more segments
                        if j < n:
                            dp[i][j+1] = True
                    else:
                        if j < n:
                            ptoks = tokenize(pseg)
                            if seg_match(ptoks, path_segs[j]):
                                dp[i+1][j+1] = True
        return dp[m][n]

    return match_segments(split_pattern(pattern), split_path(path))