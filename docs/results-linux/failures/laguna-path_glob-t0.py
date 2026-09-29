def match(pattern, path):
    def parse_pattern(p):
        tokens = []
        i = 0
        while i < len(p):
            if p[i] == '\\' and i + 1 < len(p):
                tokens.append(('literal', p[i + 1]))
                i += 2
            elif p[i] == '[':
                j = i + 1
                negated = False
                if j < len(p) and p[j] == '!':
                    negated = True
                    j += 1
                chars = []
                while j < len(p) and p[j] != ']':
                    if j + 2 < len(p) and p[j + 1] == '-' and p[j + 2] != ']':
                        chars.append((ord(p[j]), ord(p[j + 2])))
                        j += 3
                    else:
                        chars.append(ord(p[j]))
                        j += 1
                if j < len(p) and p[j] == ']':
                    tokens.append(('class', negated, chars))
                    i = j + 1
                else:
                    tokens.append(('literal', p[i]))
                    i += 1
            elif p[i] == '?':
                tokens.append(('any',))
                i += 1
            elif p[i] == '*':
                tokens.append(('star',))
                i += 1
            elif p[i] == '/':
                tokens.append(('slash',))
                i += 1
            else:
                tokens.append(('literal', p[i]))
                i += 1
        return tokens

    def char_in_class(c, negated, chars):
        for item in chars:
            if isinstance(item, tuple):
                if item[0] <= ord(c) <= item[1]:
                    return not negated
            else:
                if ord(c) == item:
                    return not negated
        return negated

    def match_segment(seg_tokens, seg):
        if not seg_tokens:
            return not seg
        t = seg_tokens[0]
        if t[0] == 'literal':
            if not seg or seg[0] != t[1]:
                return False
            return match_segment(seg_tokens[1:], seg[1:])
        elif t[0] == 'any':
            if not seg:
                return False
            return match_segment(seg_tokens[1:], seg[1:])
        elif t[0] == 'star':
            return match_segment(seg_tokens[1:], seg) or (seg and match_segment(seg_tokens, seg[1:]))
        elif t[0] == 'class':
            if not seg:
                return False
            if char_in_class(seg[0], t[1], t[2]):
                return match_segment(seg_tokens[1:], seg[1:])
            return False
        return False

    def match_pattern(tokens, segments):
        if not tokens and not segments:
            return True
        if not tokens:
            return False
        if not segments:
            for t in tokens:
                if t[0] != 'star':
                    return False
            return True
        t = tokens[0]
        if t[0] == 'slash':
            return False
        elif t[0] == 'star':
            rest_tokens = tokens[1:]
            for i in range(len(segments) + 1):
                if match_pattern(rest_tokens, segments[i:]):
                    return True
            return False
        elif t[0] == 'class':
            if segments and char_in_class(segments[0], t[1], t[2]):
                return match_pattern(tokens[1:], segments[1:])
            return False
        elif t[0] == 'literal':
            if segments and segments[0] == t[1]:
                return match_pattern(tokens[1:], segments[1:])
            return False
        elif t[0] == 'any':
            if segments:
                return match_pattern(tokens[1:], segments[1:])
            return False
        return False

    path_segments = path.split('/')
    pattern_tokens = parse_pattern(pattern)
    return match_pattern(pattern_tokens, path_segments)