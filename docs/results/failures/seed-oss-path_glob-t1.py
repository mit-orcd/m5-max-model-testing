def match(pattern, path):
    def _tokenize(s):
        tokens = []
        i = 0
        n = len(s)
        while i < n:
            c = s[i]
            if c == '\\' and i + 1 < n:
                tokens.append(('LITERAL', s[i + 1]))
                i += 2
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
                        tokens.append(('LITERAL', '['))
                        i += 1
                    else:
                        chars = set(s[i + 1:j])
                        if negated:
                            chars.add('/')
                        tokens.append(('CHAR_CLASS', chars, negated))
                        i = j + 1
                else:
                    tokens.append((c,))
                    i += 1
            else:
                tokens.append(('LITERAL', c))
                i += 1
        return tokens

    def _split(s):
        parts = []
        start = 0
        for i, c in enumerate(s):
            if c == '/':
                if start < i:
                    parts.append(s[start:i])
                parts.append('')
                start = i + 1
        if start < len(s):
            parts.append(s[start:])
        return parts

    def _compile(pat_parts):
        compiled = []
        i = 0
        n = len(pat_parts)
        while i < n:
            part = pat_parts[i]
            if part == '**':
                compiled.append(('STAR2',))
                i += 1
            elif part == '*':
                compiled.append(('STAR1',))
                i += 1
            else:
                tokens = _tokenize(part)
                compiled.append(('SEG', tokens))
                i += 1
        return compiled

    def _match_segment(tokens, seg):
        if not tokens and not seg:
            return True
        t_iter = iter(tokens)
        s_iter = iter(seg)
        t_cache = []
        s_cache = []

        def _pop_t():
            return t_cache.pop() if t_cache else next(t_iter, None)

        def _pop_s():
            return s_cache.pop() if s_cache else next(s_iter, None)

        while True:
            t = _pop_t()
            s = _pop_s()
            if t is None and s is None:
                return True
            if t is None or s is None:
                return False

            t_type = t[0] if isinstance(t, tuple) else t
            if t_type == 'LITERAL':
                if t[1] != s:
                    return False
            elif t_type == '?':
                if s == '/':
                    return False
            elif t_type == 'CHAR_CLASS':
                cc_chars, cc_neg = t[1], t[2]
                if (s in cc_chars) == cc_neg:
                    return False
            else:
                return False

    def _match_segments(compiled, path_segs):
        comp_iter = iter(compiled)
        path_iter = iter(path_segs)
        comp_cache = []
        path_cache = []

        def _next_comp():
            return comp_cache.pop() if comp_cache else next(comp_iter, None)

        def _next_path():
            return path_cache.pop() if path_cache else next(path_iter, None)

        def _save_comp(c):
            comp_cache.append(c)

        def _save_path(p):
            path_cache.append(p)

        while True:
            c = _next_comp()
            p = _next_path()

            if c is None and p is None:
                return True
            if c is None:
                return False
            if p is None:
                if c[0] == 'STAR2':
                    return all(_next_comp() is None for _ in range(len(comp_cache) + 1))
                return False

            c_type = c[0]
            if c_type == 'STAR2':
                if p == '' and len(path_segs) == 1:
                    return all(_next_comp() is None for _ in range(len(comp_cache) + 1))
                next_c = _next_comp()
                if next_c is not None and next_c[0] != 'STAR2' and _match_segments([next_c], [p]):
                    return _match_segments(compiled, path_segs)
                save = (c, p)
                _save_comp(c)
                _save_path(p)
            elif c_type == 'STAR1':
                if p == '':
                    _save_comp(c)
                    _save_path(p)
                else:
                    if not _match_segment(c[1], p):
                        _save_comp(c)
                        _save_path(p)
                    else:
                        if _next_comp() is None:
                            return not _next_path() is not None
            elif c_type == 'SEG':
                if not _match_segment(c[1], p):
                    return False
            else:
                return False
        return False

    pat_segs = _split(pattern)
    path_segs = _split(path)
    if any(p == '' for p in pat_segs[:-1] + pat_segs[-1:]) and any(p == '' for p in path_segs[:-1] + path_segs[-1:]):
        return False
    compiled = _compile(pat_segs)
    return _match_segments(compiled, path_segs)