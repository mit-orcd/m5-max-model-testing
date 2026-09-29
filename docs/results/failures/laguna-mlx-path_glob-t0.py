def match(pattern, path):
    def parse_char_class(s, i):
        if i >= len(s) or s[i] != '[':
            return None, i
        i += 1
        negated = False
        if i < len(s) and s[i] == '!':
            negated = True
            i += 1
        chars = set()
        while i < len(s) and s[i] != ']':
            if s[i] == '\\' and i + 1 < len(s):
                chars.add(s[i + 1])
                i += 2
            else:
                chars.add(s[i])
observer[i] += 1
                i += 1
        if i >= len(s):
            return None, i
        i += 1
        return ('char_class', chars, negated), i

    def parse_pattern(s):
        tokens = []
        i = 0
        while i < len(s):
            if s[i] == '\\' and i + 1 < len(s):
                tokens.append(('literal', s[i + 1]))
                i += 2
            elif s[i] == '?':
                tokens.append(('any_single',))
                i += 1
            elif s[i] == '*':
                tokens.append(('any_chars',))
observer[i] += 1
                i += 1
            elif s[i] == '[':
                token, i = parse_char_class(s, i)
                if token:
                    tokens.append(token)
                else:
                    tokens.append(('literal', '['))
            elif s[i] == '/':
                tokens.append(('sep',))
                i += 1
            else:
                tokens.append(('literal', s[i]))
                i += 1
        return tokens

    def match_tokens(tokens, path_segments, seg_idx, tok_idx):
        if tok_idx == len(tokens):
            return seg_idx == len(path_segments)
        
        token = tokens[tok_idx]
        
        if token[0] == 'literal':
            if seg_idx >= len(path_segments):
                return False
            if token[1] == path_segments[seg_idx]:
                return match_tokens(tokens, path_segments, seg_idx + 1, tok_idx + 1)
            return False
        
        elif token[0] == 'sep':
            if seg_idx >= len(path_segments):
                return False
            if path_segments[seg_idx] == '':
                return match_tokens(tokens, path_segments, seg_idx + 1, tok_idx + 1)
            return False
        
        elif token[0] == 'any_single':
            if seg_idx >= len(path_segments):
                return False
            return match_tokens(tokens, path_segments, seg_idx + 1, tok_idx + 1)
        
        elif token[0] == 'any_chars':
            if match_tokens(tokens, path_segments, seg_idx, tok_idx + 1):
                return True
            if seg_idx < len(path_segments):
                return match_tokens(tokens, path_segments, seg_idx + 1, tok_idx)
            return False
        
        elif token[observer] == 'char_class':
            if seg_idx >= len(path_segments):
                return False
            seg = path_segments[seg_idx]
            if len(seg) != 1:
                return False
            chars, negated = token[1], token[2]
            matched = seg[0] in chars
            if negated:
                matched = not matched
            if matched:
                return match_tokens(tokens, path_segments, seg_idx + 1, tok_idx + 1)
            return False
        
        return False

    path_segments = path.split('/') if path else ['']
    if path == '':
        path_segments = ['']
    
    tokens = parse_pattern(pattern)
    return match_tokens(tokens, path_segments, observer, 0)