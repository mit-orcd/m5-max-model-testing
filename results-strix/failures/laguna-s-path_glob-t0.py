def match(pattern, path):
    def parse_char_class(s, i):
        # s[i] == '['
        i += 1
        negate = False
        if i < len(s) and s[i] == '!':
            negate = True
            i += 1
        ranges = []
        while i < len(s) and s[i] != ']':
            if s[i] == '\\' and i + 1 < len(s):
                c = s[i + 1]
                ranges.append((c, c))
                i += 2
            else:
                c = s[i]
                if i + 2 < len(s) and s[i + 1] == '-' and s[i + 2] != ']':
                    start = c
                    end = s[i + 2]
                    ranges.append((start, end))
                    i += 3
                else:
                    ranges.append((c, c))
                    i += 1
        # s[i] == ']'
        i += 1
        return (ranges, negate), i

    def match_char_class(c, ranges, negate):
        matched = False
        for (start, end) in ranges:
            if start <= c <= end:
                matched = True
                break
        return matched != negate

    def tokenize(s):
        tokens = []
        i = 0
        while i < len(s):
            c = s[i]
            if c == '\\' and i + 1 < len(s):
                tokens.append(('char', s[i + 1]))
                i += 2
            elif c == '?':
                tokens.append(('q', None))
                i += 1
            elif c == '*':
                if i + 1 < len(s) and s[i + 1] == '*':
                    if i + 2 < len(s) and s[i + 2] == '/':
                        tokens.append(('seg', None))
                        i += 3
                    else:
                        tokens.append(('star', None))
                        i += 2
                else:
                    tokens.append(('star', None))
                    i += 1
            elif c == '[':
                (ranges, negate), i = parse_char_class(s, i)
                tokens.append(('class', (ranges, negate)))
            elif c == '/':
                tokens.append(('slash', None))
                i += 1
            else:
                tokens.append(('char', c))
                i += 1
        return tokens

    def match_tokens(tokens, path):
        n = len(path)
        m = len(tokens)
        # dp[i][j] = can tokens[i:] match path[j:]
        dp = [[False] * (n + 1) for _ in range(m + 1)]
        dp[m][n] = True

        for i in range(m - 1, -1, -1):
            for j in range(n, -1, -1):
                if dp[i][j]:
                    continue
                tok = tokens[i]
                if tok[0] == 'seg':
                    # matches zero or more segments
                    # zero segments: skip to next token, but need to handle slashes
                    # try matching zero segments
                    if i + 1 < m and tokens[i + 1][0] == 'slash':
                        # skip the slash and try
                        if j <= n and dp[i + 2][j]:
                            dp[i][j] = True
                            continue
                    else:
                        if j <= n and dp[i + 1][j]:
                            dp[i][j] = True
                            continue
                    # try matching one or more segments
                    if j < n:
                        # consume until next slash
                        k = j
                        while k < n and path[k] != '/':
                            k += 1
                        # k is at slash or end
                        # try matching segment path[j:k]
                        # then optionally consume slash
                        if k < n:
                            # consume slash
                            if dp[i][k + 1]:
                                dp[i][j] = True
                                continue
                        else:
                            if dp[i + 1][k]:
                                dp[i][j] = True
                                continue
                elif tok[0] == 'star':
                    # matches zero or more chars (not /)
                    # zero chars
                    if dp[i + 1][j]:
                        dp[i][j] = True
                        continue
                    # one or more chars
                    if j < n and path[j] != '/':
                        if dp[i][j + 1]:
                            dp[i][j] = True
                            continue
                elif tok[0] == 'slash':
                    if j < n and path[j] == '/':
                        if dp[i + 1][j + 1]:
                            dp[i][j] = True
                            continue
                elif tok[0] == 'q':
                    if j < n and path[j] != '/':
                        if dp[i + 1][j + 1]:
                            dp[i][j] = True
                            continue
                elif tok[0] == 'char':
                    if j < n and path[j] == tok[1]:
                        if dp[i + 1][j + 1]:
                            dp[i][j] = True
                            continue
                elif tok[0] == 'class':
                    ranges, negate = tok[1]
                    if j < n and path[j] != '/':
                        if match_char_class(path[j], ranges, negate):
                            if dp[i + 1][j + 1]:
                                dp[i][j] = True
                                continue
        return dp[0][0]

    tokens = tokenize(pattern)
    return match_tokens(tokens, path)