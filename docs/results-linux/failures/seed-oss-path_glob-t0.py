def match(pattern, path):
    def tokenize_segment(pattern_segment):
        tokens = []
        i = 0
        n = len(pattern_segment)
        while i < n:
            if pattern_segment[i] == '\\':
                if i + 1 < n:
                    tokens.append(('literal', pattern_segment[i+1]))
                    i += 2
                else:
                    tokens.append(('literal', '\\'))
                    i += 1
            elif pattern_segment[i] == '?':
                tokens.append(('any',))
                i += 1
            elif pattern_segment[i] == '*':
                tokens.append(('star',))
                i += 1
            elif pattern_segment[i] == '[':
                i += 1
                negated = False
                if i < n and pattern_segment[i] == '!':
                    negated = True
                    i += 1
                ranges = []
                while i < n and pattern_segment[i] != ']':
                    if pattern_segment[i] == '\\':
                        if i + 1 < n:
                            char = pattern_segment[i+1]
                            ranges.append((char, char))
                            i += 2
                        else:
                            ranges.append(('\\', '\\'))
                            i += 1
                    else:
                        char = pattern_segment[i]
                        ranges.append((char, char))
                        i += 1
                if i < n:
                    i += 1
                tokens.append(('charclass', negated, ranges))
            else:
                tokens.append(('literal', pattern_segment[i]))
                i += 1
        return tokens

    def matches_token(token, c):
        if token[0] == 'any':
            return True
        elif token[0] == 'charclass':
            negated, ranges = token[1], token[2]
            in_class = False
            for start, end in ranges:
                if start == end:
                    if c == start:
                        in_class = True
                        break
                else:
                    if ord(start) <= ord(c) <= ord(end):
                        in_class = True
                        break
            return not in_class if negated else in_class
        elif token[0] == 'literal':
            return token[1] == c
        return False

    def segment_match(pattern_segment, path_segment):
        tokens = tokenize_segment(pattern_segment)
        m, n = len(tokens), len(path_segment)
        dp = [[False] * (n + 1) for _ in range(m + 1)]
        dp[0][0] = True

        for ti in range(1, m + 1):
            if tokens[ti-1] == ('star',):
                dp[ti][0] = dp[ti-1][0]
            else:
                break

        for ti in range(1, m + 1):
            token = tokens[ti-1]
            for si in range(1, n + 1):
                if token == ('star',):
                    dp[ti][si] = dp[ti-1][si] or dp[ti][si-1]
                else:
                    if matches_token(token, path_segment[si-1]) and dp[ti-1][si-1]:
                        dp[ti][si] = True
        return dp[m][n]

    def match_segments(pat_segs, path_segs):
        i = j = 0
        len_p, len_q = len(pat_segs), len(path_segs)

        while i < len_p and j < len_q:
            p = pat_segs[i]
            if p == '**':
                for k in range(j, len_q + 1):
                    if match_segments(pat_segs[i+1:], path_segs[k:]):
                        return True
                return False
            else:
                if not segment_match(p, path_segs[j]):
                    return False
                i, j = i + 1, j + 1

        if i >= len_p:
            return j >= len_q
        for p in pat_segs[i:]:
            if p != '**':
                return False
        return True

    pat_segs = pattern.split('/')
    path_segs = path.split('/')
    return match_segments(pat_segs, path_segs)