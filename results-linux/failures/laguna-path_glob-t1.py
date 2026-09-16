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
                if j < len(p) and p[j] == '!':
                    negation = True
                    j += 1
                else:
                    negation = False
                if j >= len(p):
                    tokens.append(('literal', '['))
                    i += 1
                    continue
                if p[j] == ']':
                    tokens.append(('literal', '['))
                    i += 1
                    continue
                chars = []
                while j < len(p) and p[j] != ']':
                    if j + 2 < len(p) and p[j + 1] == '-':
                        chars.append((p[j], p[j + 2]))
                        j += 3
                    else:
                        chars.append(p[j])
                        j += 1
                if j >= len(p):
                    tokens.append(('literal', '['))
                    i += 1
                    continue
                j += 1
                tokens.append(('char_class', chars, negation))
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

    def char_matches(c, char_class):
        chars, negation = char_class
        matched = False
        for item in chars:
            if isinstance(item, tuple):
                if item[0] <= c <= item[1]:
                    matched = True
                    break
            else:
                if c == item:
                    matched = True
                    break
        return matched != negation

    def parse_path(path):
        return path.split('/')

    def match_tokens(tokens, segments, seg_idx, tok_idx):
        if tok_idx == len(tokens):
            return seg_idx == len(segments)
        if seg_idx > len(segments):
            return False
        token = tokens[tok_idx]
        if token[0] == 'slash':
            if seg_idx < len(segments) and segments[seg_idx] == '':
                return match_tokens(tokens, segments, seg_idx + 1, tok_idx + 1)
            return False
        if token[0] == 'literal':
            if seg_idx < len(segments) and segments[seg_idx] == token[1]:
                return match_tokens(tokens, segments, seg_idx + 1, tok_idx + 1)
            return False
        if token[0] == 'question':
            if seg_idx < len(segments) and len(segments[seg_idx]) > 0 and segments[seg_idx][0] != '/':
                return match_tokens(tokens, segments, seg_idx, tok_idx + 1)
            return False
        if token[0] == 'star':
            if match_tokens(tokens, segments, seg_idx, tok_idx + 1):
                return True
            if seg_idx < len(segments) and len(segments[seg_idx]) > 0 and segments[seg_idx][0] != '/':
                if match_tokens(tokens, segments, seg_idx + 1, tok_idx):
                    return True
            return False
        if token[0] == 'char_class':
            if seg_idx < len(segments) and len(segments[seg_idx]) > 0 and segments[seg_idx][0] != '/':
                c = segments[seg_idx][0]
                if char_matches(c, token[1:]):
                    return match_tokens(tokens, segments, seg_idx + 1, tok_idx + 1)
            return False
        return False

    def match_segment(tokens, segment):
        if not tokens:
            return len(segment) == 0
        if len(tokens) == 1 and tokens[0][0] == 'star':
            return True
        return match_tokens(tokens, [segment], 0, 0)

    def process_double_star(tokens):
        new_tokens = []
        i = 0
        while i < len(tokens):
            if tokens[i][0] == 'star' and i + 1 < len(tokens) and tokens[i + 1][0] == 'slash':
                new_tokens.append(('double_star',))
                i += 2
                while i < len(tokens) and tokens[i][0] == 'slash':
                    new_tokens.append(('slash',))
                    i += 1
            else:
                new_tokens.append(tokens[i])
                i += 1
        return new_tokens

    tokens = parse_pattern(pattern)
    tokens = process_double_star(tokens)
    segments = parse_path(path)
    
    if not tokens:
        return len(segments) == 0 or (len(segments) == 1 and segments[0] == '')
    
    return match_tokens(tokens, segments, 0, 0)