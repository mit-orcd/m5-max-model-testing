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
                    if j + 2 < len(p) and p[j + 1] == '-':
                        chars.append((p[j], p[j + 2]))
                        j += 3
                    else:
                        chars.append((p[j], None))
                        j += 1
                if j < len(p) and p[j] == ']':
                    j += 1
                tokens.append(('class', chars, negated))
                i = j
            elif p[i] == '?':
                tokens.append(('question',))
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

    def char_in_class(c, chars, negated):
        for start, end in chars:
            if end is None:
                if c == start:
                    return not negated
            else:
                if start <= c <= end:
                    return not negated
        return negated

    def match_segment(seg, tokens, pos):
        if pos == len(tokens):
            return True
        token = tokens[pos]
        if token[0] == 'literal':
            if seg and seg[0] == token[1]:
                return match_segment(seg[1:], tokens, pos + 1)
            return False
        elif token[0] == 'class':
            if seg and char_in_class(seg[0], token[1], token[2]):
                return match_segment(seg[1:], tokens, pos + 1)
            return False
        elif token[0] == 'question':
            if seg:
                return match_segment(seg[1:], tokens, pos + 1)
            return False
        elif token[0] == 'star':
            if match_segment(seg, tokens, pos + 1):
                return True
            if seg:
                return match_segment(seg[1:], tokens, pos)
            return False
        return False

    def match_path(path, pattern):
        path_segs = path.split('/')
        pat_segs = pattern.split('/')
        if len(pat_segs) == 0:
            return len(path_segs) == 0
        if pat_segs[0] == '**':
            if len(path_segs) == 0:
                return len(pat_segs) == 1
            if match_path('/'.join(path_segs[1:]), '/'.join(pat_segs[1:])):
                return True
            return match_path('/'.join(path_segs[:-1]), '/'.join(pat_segs[:-1])) and len(pat_segs) > 1
        else:
            if len(path_segs) == 0:
                return False
            pat_tokens = parse_pattern(pat_segs[0])
            if not match_segment(path_segs[0], pat_tokens, 0):
                return False
            return match_path('/'.join(path_segs[1:]), '/'.join(pat_segs[1:]))

    return match_path(path, pattern)