def match(pattern, path):
    def parse_pattern(p):
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
                negate = False
                if j < len(p) and p[j] == '!':
                    negate = True
                    j += 1
                chars = set()
                while j < len(p) and p[j] != ']':
                    if j + 2 < len(p) and p[j + 1] == '-':
                        chars.add(p[j])
                        chars.add(p[j + 2])
                        j += 3
                    else:
                        chars.add(p[j])
                        j += 1
                if j < len(p) and p[j] == ']':
                    j += 1
                tokens.append(('class', negate, chars))
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

    def parse_path(path):
        return path.split('/')

    pattern_tokens = parse_pattern(pattern)
    path_segments = parse_path(path)

    def match_tokens(tokens, segments, seg_idx, token_idx):
        if token_idx == len(tokens) and seg_idx == len(segments):
            return True
        if token_idx >= len(tokens):
            return False

        token = tokens[token_idx]

        if token[0] == 'slash':
            if seg_idx >= len(segments):
                return False
            return match_tokens(tokens, segments, seg_idx + 1, token_idx + 1)

        if token[0] == 'star':
            if match_tokens(tokens, segments, seg_idx, token_idx + 1):
                return True
            if seg_idx < len(segments):
                return match_tokens(tokens, segments, seg_idx, token_idx + 1) or match_tokens(tokens, segments, seg_idx + 1, token_idx)
            return False

        if token[0] == 'question':
            if seg_idx >= len(segments):
                return False
            if '/' in segments[seg_idx]:
                return False
            return match_tokens(tokens, segments, seg_idx + 1, token_idx + 1)

        if token[0] == 'class':
            negate, chars = token[1], token[2]
            if seg_idx >= len(segments):
                return False
            seg = segments[seg_idx]
            if len(seg) != 1:
                return False
            matched = seg in chars
            if negate:
                matched = not matched
            if matched:
                return match_tokens(tokens, segments, seg_idx + 1, token_idx + 1)
            return False

        if token[0] == 'literal':
            lit = token[1]
            if seg_idx >= len(segments):
                return False
            seg = segments[seg_idx]
            if seg == lit:
                return match_tokens(tokens, segments, seg_idx + 1, token_idx + 1)
            return False

        return False

    return match_tokens(pattern_tokens, path_segments, 0, 0)