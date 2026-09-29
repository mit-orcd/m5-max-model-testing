def match(pattern, path):
    def parse_pattern(p):
        i = 0
        n = len(p)
        tokens = []
        while i < n:
            c = p[i]
            if c == '\\':
                if i + 1 < n:
                    tokens.append(('lit', p[i + 1]))
                    i += 2
                else:
                    tokens.append(('lit', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('q',))
                i += 1
            elif c == '*':
                if i + 1 < n and p[i + 1] == '*':
                    tokens.append(('starstar',))
                    i += 2
                else:
                    tokens.append(('star',))
                    i += 1
            elif c == '[':
                j = i + 1
                neg = False
                if j < n and p[j] == '!':
                    neg = True
                    j += 1
                if j < n and p[j] == ']':
                    j += 1
                chars = set()
                while j < n and p[j] != ']':
                    if p[j] == '\\' and j + 1 < n:
                        chars.add(p[j + 1])
                        j += 2
                    elif j + 2 < n and p[j + 1] == '-' and p[j + 2] != ']':
                        chars.update(range(ord(p[j]), ord(p[j + 2]) + 1))
                        j += 3
                    else:
                        chars.add(p[j])
                        j += 1
                if j >= n:
                    tokens.append(('lit', '['))
                    i = j
                else:
                    tokens.append(('class', chars, neg))
                    i = j + 1
            else:
                tokens.append(('lit', c))
                i += 1
        return tokens

    def match_segment(tokens, seg, t_idx):
        if t_idx >= len(tokens):
            return seg == ""
        tok = tokens[t_idx]
        if tok[0] == 'lit':
            if seg and seg[0] == tok[1]:
                return match_segment(tokens, seg[1:], t_idx + 1)
            return False
        elif tok[0] == 'q':
            if seg and seg[0] != '/':
                return match_segment(tokens, seg[1:], t_idx + 1)
            return False
        elif tok[0] == 'star':
            if not seg:
                return match_segment(tokens, "", t_idx + 1)
            if seg[0] == '/':
                return False
            return match_segment(tokens, seg[1:], t_idx) or match_segment(tokens, seg, t_idx + 1)
        elif tok[0] == 'class':
            chars, neg = tok[1], tok[2]
            if seg and seg[0] != '/':
                in_class = seg[0] in chars
                if (not neg and in_class) or (neg and not in_class):
                    return match_segment(tokens, seg[1:], t_idx + 1)
            return False
        return False

    def match_path(tokens, path, t_idx, p_idx):
        if t_idx >= len(tokens):
            return p_idx >= len(path)
        tok = tokens[t_idx]
        if tok[0] == 'starstar':
            if t_idx + 1 >= len(tokens):
                return p_idx >= len(path)
            if p_idx >= len(path):
                return match_path(tokens, path, t_idx + 1, p_idx)
            if path[p_idx] == '/':
                return match_path(tokens, path, t_idx + 1, p_idx + 1)
            return match_path(tokens, path, t_idx, p_idx + 1) or match_path(tokens, path, t_idx + 1, p_idx)
        if tok[0] == 'lit' and tok[1] == '/':
            if p_idx < len(path) and path[p_idx] == '/':
                return match_path(tokens, path, t_idx + 1, p_idx + 1)
            return False
        if tok[0] == 'star':
            if p_idx >= len(path):
                return match_path(tokens, path, t_idx + 1, p_idx)
            if path[p_idx] == '/':
                return match_path(tokens, path, t_idx + 1, p_idx)
            return match_path(tokens, path, t_idx, p_idx + 1) or match_path(tokens, path, t_idx + 1, p_idx)
        if tok[0] == 'q':
            if p_idx < len(path) and path[p_idx] != '/':
                return match_path(tokens, path, t_idx + 1, p_idx + 1)
            return False
        if tok[0] == 'class':
            chars, neg = tok[1], tok[2]
            if p_idx < len(path) and path[p_idx] != '/':
                in_class = path[p_idx] in chars
                if (not neg and in_class) or (neg and not in_class):
                    return match_path(tokens, path, t_idx + 1, p_idx + 1)
            return False
        return False

    tokens = parse_pattern(pattern)
    return match_path(tokens, path, 0, 0)