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
                if c == '?':
                    tokens.append(('QMARK', None))
                elif c == '*':
                    tokens.append(('STAR', None))
                elif c == '[':
                    i += 1
                    negated = False
                    if i < n and s[i] == '!':
                        negated = True
                        i += 1
                    elif i < n and s[i] == '^':
                        negated = True
                        i += 1
                    chars = []
                    while i < n and s[i] != ']':
                        if i + 1 < n and s[i] == '\\' and s[i + 1] != ']':
                            chars.append(s[i + 1])
                            i += 2
                        else:
                            chars.append(s[i])
                            i += 1
                    if i < n and s[i] == ']':
                        i += 1
                    tokens.append(('CHARCLASS', (negated, chars)))
            else:
                tokens.append(('LITERAL', c))
                i += 1
        return tokens

    def _split(p):
        parts = []
        start = 0
        n = len(p)
        i = 0
        while i < n:
            if p[i] == '\\':
                if i + 1 < n:
                    if i > start:
                        parts.append(p[start:i])
                    parts.append(p[i + 1])
                    start = i + 2
                    i += 2
                else:
                    if i > start:
                        parts.append(p[start:i])
                    parts.append('\\')
                    start = i + 1
                    i += 1
            elif p[i] == '/':
                if i > start:
                    parts.append(p[start:i])
                parts.append('/')
                start = i + 1
                i += 1
            else:
                i += 1
        if start < n:
            parts.append(p[start:n])
        normalized = []
        i = 0
        while i < len(parts):
            if parts[i] == '/':
                if not normalized or normalized[-1] != '/':
                    normalized.append('/')
            else:
                if normalized and normalized[-1] == '/':
                    pass
                else:
                    normalized.append(parts[i])
            i += 1
        final = []
        for part in normalized:
            if part != '/':
                final.append(part)
            else:
                if final and final[-1] != '':
                    final.append('')
        if final and final[-1] == '':
            final.pop()
        return final

    def _match_segment(tokens, seg, start, end):
        if start >= end:
            return end == len(tokens)
        t = tokens[start]
        if t[0] == 'LITERAL':
            if end > len(seg) or seg[end - 1] != t[1]:
                return False
            return _match_segment(tokens, seg, start + 1, end + 1)
        elif t[0] == 'QMARK':
            if end >= len(seg) or seg[end - 1] == '/':
                return False
            return _match_segment(tokens, seg, start + 1, end + 1)
        elif t[0] == 'STAR':
            for e in range(end, len(seg) + 1):
                if _match_segment(tokens, seg, start + 1, e):
                    return True
            return _match_segment(tokens, seg, start + 1, end)
        elif t[0] == 'CHARCLASS':
            negated, chars = t[1]
            if end >= len(seg):
                return False
            c = seg[end - 1]
            if c == '/':
                return False
            match = c in chars
            if negated:
                match = not match
            if not match:
                return False
            return _match_segment(tokens, seg, start + 1, end + 1)
        return False

    def _match_star_segments(patt_parts, path_parts, p_start, p_end, s_start, s_end):
        if p_end == len(patt_parts):
            return s_end == len(path_parts)
        if s_end == len(path_parts):
            return all(patt_parts[i] == '**' for i in range(p_start, p_end))
        if p_end < len(patt_parts) and patt_parts[p_end] != '**':
            for s in range(s_start, s_end + 1):
                if _match_part(patt_parts[p_end], path_parts[s]) and _match_star_segments(patt_parts, path_parts, p_end + 1, p_end + 1, s + 1, len(path_parts)):
                    return True
            return False
        return _match_star_segments(patt_parts, path_parts, p_start, p_end + 1, s_start, s_end + 1) or _match_star_segments(patt_parts, path_parts, p_start, p_end, s_start + 1, s_end)

    def _match_part(patt_part, path_part):
        tokens = _tokenize(patt_part)
        return _match_segment(tokens, path_part, 0, 0)

    pattern_parts = _split(pattern)
    path_parts = _split(path)

    p = 0
    s = 0
    while p < len(pattern_parts) and s < len(path_parts):
        if pattern_parts[p] == '**':
            return _match_star_segments(pattern_parts, path_parts, p, p, s, len(path_parts) - 1)
        if not _match_part(pattern_parts[p], path_parts[s]):
            return False
        p += 1
        s += 1

    while p < len(pattern_parts):
        if pattern_parts[p] != '**':
            return False
        p += 1

    return p == len(pattern_parts) and s == len(path_parts)