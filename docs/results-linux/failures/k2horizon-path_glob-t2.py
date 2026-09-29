def match(pattern, path):
    def parse_class(s, i):
        # s[i] == '['
        i += 1
        neg = False
        if i < len(s) and s[i] == '!':
            neg = True
            i += 1
        chars = set()
        while i < len(s) and s[i] != ']':
            if i + 2 < len(s) and s[i+1] == '-':
                a, b = s[i], s[i+2]
                if a <= b:
                    for c in range(ord(a), ord(b) + 1):
                        chars.add(chr(c))
                i += 3
            else:
                chars.add(s[i])
                i += 1
        if i < len(s) and s[i] == ']':
            i += 1
        if not neg:
            return chars, i
        return None, i  # negation handled by caller

    def parse_segment(seg):
        # returns list of tokens: ('char', c), ('q',), ('star',), ('class', chars_or_None_for_neg, neg), ('bs', c)
        tokens = []
        i = 0
        while i < len(seg):
            c = seg[i]
            if c == '\\' and i + 1 < len(seg):
                tokens.append(('char', seg[i+1]))
                i += 2
            elif c == '?':
                tokens.append(('q',))
                i += 1
            elif c == '*':
                tokens.append(('star',))
                i += 1
            elif c == '[':
                chars, j = parse_class(seg, i)
                if j > i:  # valid class
                    # determine negation
                    k = i + 1
                    neg = False
                    if k < len(seg) and seg[k] == '!':
                        neg = True
                    tokens.append(('class', chars, neg))
                    i = j
                else:
                    tokens.append(('char', '['))
                    i += 1
            else:
                tokens.append(('char', c))
                i += 1
        return tokens

    def seg_match(tokens, text):
        # dp over tokens and text positions
        n = len(tokens)
        m = len(text)
        # dp[i][j] = tokens[i:] matches text[j:]
        dp = [[False] * (m + 1) for _ in range(n + 1)]
        dp[n][m] = True
        for i in range(n - 1, -1, -1):
            t = tokens[i]
            if t[0] == 'star':
                for j in range(m, -1, -1):
                    dp[i][j] = dp[i+1][j] or (j < m and dp[i][j+1])
            elif t[0] == 'q':
                for j in range(m):
                    dp[i][j] = dp[i+1][j+1]
            elif t[0] == 'char':
                for j in range(m):
                    if text[j] == t[1]:
                        dp[i][j] = dp[i+1][j+1]
            elif t[0] == 'class':
                chars, neg = t[1], t[2]
                for j in range(m):
                    if text[j] == '/':
                        continue
                    hit = (text[j] in chars) if chars is not None else (text[j] not in chars)
                    if neg:
                        hit = not hit
                    if hit:
                        dp[i][j] = dp[i+1][j+1]
        return dp[0][0]

    # parse pattern into segments, marking '**' segments
    pat_segs = pattern.split('/')
    pat_tokens = [parse_segment(s) for s in pat_segs]
    is_starstar = [s == '**' for s in pat_segs]

    p = len(pat_tokens)
    text_segs = path.split('/')
    t = len(text_segs)

    # dp over pattern segment index and text segment index
    # dp[i][j] = pattern segments i: match text segments j:
    dp = [[False] * (t + 1) for _ in range(p + 1)]
    dp[p][t] = True
    for i in range(p - 1, -1, -1):
        if is_starstar[i]:
            for j in range(t, -1, -1):
                dp[i][j] = dp[i+1][j] or (j < t and dp[i][j+1])
        else:
            for j in range(t):
                if seg_match(pat_tokens[i], text_segs[j]):
                    dp[i][j] = dp[i+1][j+1]
    return dp[0][0]