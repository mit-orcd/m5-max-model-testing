def match(pattern, path):
    def parse_pattern(p):
        tokens = []
        i = 0
        n = len(p)
        while i < n:
            if p[i] == '\\' and i + 1 < n:
                tokens.append(('literal', p[i + 1]))
                i += 2
            elif p[i] == '[':
                j = i + 1
                negate = False
                if j < n and p[j] == '!':
                    negate = True
                    j += 1
                chars = []
                while j < n and p[j] != ']':
                    if j + 2 < n and p[j+1] == '-' and p[j+2] != ']':
                        start = ord(p[j])
                        end = ord(p[j+2])
                        chars.extend(range(start, end + 1))
                        j += 3
                    else:
                        chars.append(ord(p[j]))
                        j += 1
                if j < n and p[j] == ']':
                    tokens.append(('class', set(chars), negate))
                    i = j + 1
                else:
                    tokens.append(('literal', '['))
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

    def tokenize_path(path):
        return path.split('/')

    tokens = parse_pattern(pattern)
    path_segments = tokenize_path(path)

    def match_tokens(token_idx, seg_idx, in_segment):
        if token_idx == len(tokens) and seg_idx == len(path_segments):
            return True
        if token_idx >= len(tokens):
            return False
        if seg_idx >= len(path_segments):
            for t in range(token_idx, len(tokens)):
                if tokens[t][0] in ('star', 'slash'):
                    continue
                elif tokens[t][0] == 'any':
                    return False
                elif tokens[t][0] == 'class':
                    return False
                else:
                    return False
            return True

        token = tokens[token_idx]
        current_seg = path_segments[seg_idx] if seg_idx < len(path_segments) else ''

        if token[0] == 'slash':
            if seg_idx < len(path_segments) - 1:
                return False
            return match_tokens(token_idx + 1, seg_idx + 1, False)
        elif token[0] == 'star':
            if match_tokens(token_idx + 1, seg_idx, False):
                return True
            if seg_idx < len(path_segments):
                return match_tokens(token_idx, seg_idx + 1, False)
            return False
        elif token[0] == 'any':
            if in_segment and current_seg and current_seg[0] != '/':
                return match_tokens(token_idx + 1, seg_idx, True) and seg_idx < len(path_segments)
            return False
        elif token[0] == 'class':
            if not in_segment and seg_idx < len(path_segments):
                char = current_seg[0]
                if (char in token[1]) != token[2]:
                    return False
                return match_tokens(token_idx + 1, seg_idx, True and len(current_seg) > 1)
            return False
        elif token[0] == 'literal':
            if token[1] == '/':
                if seg_idx < len(path_segments) - 1:
                    return False
                return match_tokens(token_idx + 1, seg_idx + 1, False)
            else:
                if in_segment:
                    if seg_idx < len(path_segments) and current_seg and current_seg[0] == token[1]:
                        return match_tokens(token_idx + 1, seg_idx, True and len(current_seg) > 1)
                else:
                    if seg_idx < len(path_segments):
                        return match_tokens(token_idx + 1, seg_idx, True and len(current_seg) > 0)
                return False
        return False

    return match_tokens(0, 0, False)