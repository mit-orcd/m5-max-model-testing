def match(pattern, path):
    def compile(pat):
        segments = []
        i = 0
        n = len(pat)
        current = []
        while i < n:
            c = pat[i]
            if c == '\\':
                if i + 1 < n:
                    current.append(('lit', pat[i+1]))
                    i += 2
                    continue
                else:
                    current.append(('lit', '\\'))
                    i += 1
                    continue
            elif c == '?':
                current.append(('any',))
                i += 1
            elif c == '*':
                if i + 1 < n and pat[i+1] == '*':
                    if i + 2 < n and pat[i+2] == '/':
                        # **/ -> match zero or more segments
                        if current:
                            segments.append(current)
                            current = []
                        segments.append(('**',))
                        i += 3
                        continue
                    else:
                        # ** not followed by / -> treat as two stars? or as *?
                        # According to problem, '**' is only special as a segment.
                        current.append(('star',))
                        i += 1
                else:
                    current.append(('star',))
                    i += 1
            elif c == '[':
                # parse character class
                j = i + 1
                negate = False
                if j < n and pat[j] == '!':
                    negate = True
                    j += 1
                chars = set()
                ranges = []
                while j < n and pat[j] != ']':
                    if pat[j] == '\\' and j + 1 < n:
                        chars.add(pat[j+1])
                        j += 2
                    elif j + 2 < n and pat[j+1] == '-' and pat[j+2] != ']':
                        ranges.append((pat[j], pat[j+2]))
                        j += 3
                    else:
                        chars.add(pat[j])
                        j += 1
                if j >= n:
                    # invalid class, treat literally
                    current.append(('lit', '['))
                    i += 1
                    continue
                current.append(('class', negate, chars, ranges))
                i = j + 1
            elif c == '/':
                segments.append(current)
                current = []
                i += 1
            else:
                current.append(('lit', c))
                i += 1
        segments.append(current)
        return segments

    def match_seg(seg, s):
        # seg: list of tokens, s: string (no '/')
        dp = [False] * (len(s) + 1)
        dp[0] = True
        for tok in seg:
            new_dp = [False] * (len(s) + 1)
            if tok[0] == 'lit':
                for i in range(len(s)):
                    if dp[i] and s[i] == tok[1]:
                        new_dp[i+1] = True
            elif tok[0] == 'any':
                for i in range(len(s)):
                    if dp[i]:
                        new_dp[i+1] = True
            elif tok[0] == 'star':
                # matches zero or more non-slash chars
                for i in range(len(s)+1):
                    if dp[i]:
                        for k in range(i, len(s)+1):
                            new_dp[k] = True
                        break
            elif tok[0] == 'class':
                negate, chars, ranges = tok[1], tok[2], tok[3]
                for i in range(len(s)):
                    if dp[i]:
                        ch = s[i]
                        matched = ch in chars
                        if not matched:
                            for (lo, hi) in ranges:
                                if lo <= ch <= hi:
                                    matched = True
                                    break
                        if negate:
                            matched = not matched
                        if matched:
                            new_dp[i+1] = True
            dp = new_dp
            if not any(dp):
                return False
        return dp[len(s)]

    def match_segments(segs, path_segs):
        # segs: compiled pattern segments
        # path_segs: list of path segments
        m, n = len(segs), len(path_segs)
        dp = [[False]*(n+1) for _ in range(m+1)]
        dp[0][0] = True
        for i in range(m+1):
            for j in range(n+1):
                if i == 0 and j == 0:
                    continue
                if i == 0:
                    dp[i][j] = False
                elif segs[i-1] == ('**',):
                    # matches zero or more segments
                    dp[i][j] = dp[i-1][j] or (dp[i][j-1] if j > 0 else False)
                else:
                    if j > 0 and dp[i-1][j-1]:
                        dp[i][j] = match_seg(segs[i-1], path_segs[j-1])
        return dp[m][n]

    compiled = compile(pattern)
    path_segs = path.split('/')
    return match_segments(compiled, path_segs)