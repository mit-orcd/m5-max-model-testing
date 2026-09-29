def match(pattern, path):
    def _tokenize(s):
        tokens = []
        i = 0
        n = len(s)
        while i < n:
            c = s[i]
            if c == '\\':
                if i + 1 < n:
                    tokens.append(('LITERAL', s[i + 1]))
                    i += 2
                else:
                    tokens.append(('LITERAL', '\\'))
                    i += 1
            elif c in '?*[]':
                if c == '[':
                    j = i + 1
                    negated = False
                    if j < n and s[j] == '!':
                        negated = True
                        j += 1
                    while j < n and s[j] != ']':
                        j += 1
                    if j >= n:
                        tokens.append(('CHAR_CLASS', ''))
                    else:
                        chars = s[i + 1 + negated:j]
                        tokens.append(('CHAR_CLASS', (negated, chars)))
                    i = j + 1
                else:
                    tokens.append((c,))
                    i += 1
            else:
                tokens.append(('LITERAL', c))
                i += 1
        return tokens

    def _split(p):
        parts = []
        start = 0
        for i, c in enumerate(p):
            if c == '/':
                if start < i:
                    parts.append(p[start:i])
                parts.append('')
                start = i + 1
        if start <= len(p):
            parts.append(p[start:])
        return [p for p in parts if p != '']

    def _char_class_match(c, cc):
        negated, chars = cc
        for r in chars.split(':'):
            if '-' in r and len(r) == 2:
                a, b = r
                if a <= c <= b:
                    return not negated
            else:
                if c in r:
                    return not negated
        return negated

    def _match_segment(p_seg, pat_seg):
        p_len, ps_len = len(p_seg), len(pat_seg)
        dp = [[False] * (ps_len + 1) for _ in range(p_len + 1)]
        dp[0][0] = True

        for i in range(1, ps_len + 1):
            if pat_seg[i - 1][0] == '*':
                dp[0][i] = dp[0][i - 1]

        for i in range(1, p_len + 1):
            for j in range(1, ps_len + 1):
                pc = p_seg[i - 1]
                pt = pat_seg[j - 1]

                if pt[0] == '?':
                    dp[i][j] = dp[i - 1][j - 1] and pc != '/'
                elif pt[0] == '*':
                    dp[i][j] = dp[i - 1][j] or dp[i][j - 1]
                elif pt[0] == 'CHAR_CLASS':
                    dp[i][j] = dp[i - 1][j - 1] and _char_class_match(pc, pt[1])
                elif pt[0] == 'LITERAL':
                    dp[i][j] = dp[i - 1][j - 1] and pc == pt[1]
                else:
                    dp[i][j] = False

        return dp[p_len][ps_len]

    def _match_star(p_parts, p_idx, pat_segs, pat_idx):
        if pat_idx == len(pat_segs):
            return p_idx == len(p_parts)
        if p_idx == len(p_parts):
            return all(seg[0] == '*' for seg in pat_segs[pat_idx:])
        if _match(p_parts, p_idx, pat_segs, pat_idx + 1):
            return True
        return _match_star(p_parts, p_idx + 1, pat_segs, pat_idx)

    def _match(p_parts, p_idx, pat_segs, pat_idx):
        while p_idx < len(p_parts) and pat_idx < len(pat_segs):
            seg = pat_segs[pat_idx]
            if seg == ('*',):
                return _match_star(p_parts, p_idx, pat_segs, pat_idx + 1)
            if not _match_segment(p_parts[p_idx], seg):
                return False
            p_idx += 1
            pat_idx += 1
        while pat_idx < len(pat_segs):
            if pat_segs[pat_idx] != ('*',):
                return False
            pat_idx += 1
        return p_idx == len(p_parts)

    pattern_parts = _split(pattern)
    path_parts = _split(path)
    pattern_tokens = [_tokenize(part) for part in pattern_parts]
    return _match(path_parts, 0, pattern_tokens, 0)