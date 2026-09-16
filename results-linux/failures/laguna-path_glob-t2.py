def match(pattern, path):
    def parse_tokens(p):
        tokens = []
        i = 0
        while i < len(p):
            if p[i] == '\\':
                if i + 1 < len(p):
                    tokens.append(('literal', p[i + 1]))
                    i += 2
                else:
                    tokens.append(('literal', p[i]))
                    i += 1
            elif p[i] == '[':
                j = i + 1
                if j < len(p) and p[j] == '!':
                    negated = True
                    j += 1
                else:
                    negated = False
                chars = []
                while j < len(p) and p[j] != ']':
                    if j + 2 < len(p) and p[j + 1] == '-':
                        chars.append((ord(p[j]), ord(p[j + 2])))
                        j += 3
                    else:
                        chars.append((ord(p[j]), ord(p[j])))
                        j += 1
                if j < len(p) and p[j] == ']':
                    j += 1
                tokens.append(('class', chars, negated))
                i = j
            elif p[i] == '?':
                tokens.append(('any',))
                i += 1
            elif p[i] == '*':
                tokens.append(('star',))
                i += 1
            elif p[i] == '**':
                tokens.append(('dstar',))
                i += 2
            else:
                tokens.append(('literal', p[i]))
                i += 1
        return tokens

    def token_match(token, char):
        if token[0] == 'literal':
            return char == token[1]
        elif token[0] == 'any':
            return char != '/'
        elif token[0] == 'class':
            matched = False
            for (lo, hi) in token[1]:
                if lo <= ord(char) <= hi:
                    matched = True
                    break
            return matched != token[2]
        return False

    def match_segment(tokens, segment):
        n = len(tokens)
        m = len(segment)
        dp = [[False] * (m + 1) for _ in range(n + 1)]
        dp[0][0] = True
        for i in range(1, n + 1):
            if tokens[i - 1][0] == 'star':
                dp[i][0] = dp[i - 1][0]
            elif tokens[i - 1][0] == 'dstar':
                dp[i][0] = dp[i - 1][0]
        for i in range(1, n + 1):
            for j in range(1, m + 1):
                t = tokens[i - 1]
                if t[0] == 'star':
                    dp[i][j] = dp[i - 1][j] or dp[i][j - 1]
                elif t[0] == 'dstar':
                    dp[i][j] = dp[i - 1][j]
                else:
                    if token_match(t, segment[j - 1]):
                        dp[i][j] = dp[i - 1][j - 1]
        return dp[n][m]

    def match_path(tokens, path):
        segments = path.split('/')
        n = len(segments)
        m = len(tokens)
        dp = [[False] * (n + 1) for _ in range(m + 1)]
        dp[0][0] = True
        for i in range(1, m + 1):
            if tokens[i - 1][0] == 'dstar':
                dp[i][0] = dp[i - 1][0]
        for i in range(1, m + 1):
            for j in range(1, n + 1):
                t = tokens[i - 1]
                if t[0] == 'dstar':
                    dp[i][j] = dp[i - 1][j]
                    if not dp[i][j] and j > 0:
                        for k in range(j):
                            if dp[i - 1][k]:
                                dp[i][j] = True
                                break
                elif t[0] == 'star':
                    dp[i][j] = dp[i - 1][j]
                else:
                    seg = segments[j - 1]
                    if t[0] == 'literal' or t[0] == 'any' or t[0] == 'class':
                        if len(seg) == 1 and token_match(t, seg):
                            dp[i][j] = dp[i - 1][j - 1]
        return dp[m][n]

    tokens = parse_tokens(pattern)
    return match_path(tokens, path)