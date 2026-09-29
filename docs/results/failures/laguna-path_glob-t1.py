def match(pattern, path):
    def parse_char_class(s, i):
        negated = False
        if s[i] == '!':
            negated = True
            i += 1
        chars = set()
        while i < len(s) and s[i] != ']':
            if s[i] == '-' and i > 0 and i < len(s) - 1:
                start = chars.pop()
                end = s[i+1]
                chars.update(chr(c) for c in range(ord(start), ord(end) + 1))
                i += 2
            else:
                chars.add(s[i])
                i += 1
        return (chars, negated, i + 1) if i < len(s) else (chars, negated, i)
    
    def parse_segment(s, i):
        seg = []
        while i < len(s) and s[i] not in '?*[]':
            seg.append(s[i])
            i += 1
        return ('literal', ''.join(seg), i) if seg else None
    
    def parse_star(s, i):
        if i < len(s) and s[i] == '*':
            if i + 1 < len(s) and s[i+1] == '*':
                return ('double_star', i + 2)
            return ('star', i + 1)
        return None
    
    def parse_question(s, i):
        if i < len(s):
            return ('question', i + 1)
        return None
    
    def parse_bracket(s, i):
        if i + 1 < len(s):
            chars, negated, new_i = parse_char_class(s, i + 1)
            return ('char_class', chars, negated, new_i)
        return None
    
    def tokenize(s):
        tokens = []
        i = 0
        while i < len(s):
            if s[i] == '\\':
                if i + 1 < len(s):
                    tokens.append(('escape', s[i+1]))
                    i += 2
                else:
                    tokens.append(('escape', '\\'))
                    i += 1
            elif m := parse_segment(s, i):
                tokens.append(m)
            elif m := parse_star(s, i):
                tokens.append(m)
            elif m := parse_question(s, i):
                tokens.append(m)
            elif m := parse_bracket(s, i):
                tokens.append(m)
            else:
                tokens.append(('literal', s[i]))
                i += 1
        return tokens
    
    def match_char_class(tokens, pos, chars, negated):
        if pos >= len(path):
            return False
        c = path[pos]
        matched = c in chars
        return (not matched) if negated else matched
    
    def match_tokens(tokens, p_idx, t_idx):
        if t_idx == len(tokens):
            return p_idx == len(path)
        
        token = tokens[t_idx]
        
        if token[0] == 'literal':
            seg, val, _ = token
            if seg is None:
                return match_tokens(tokens, p_idx, t_idx + 1)
            if not path[p_idx:].startswith(val):
                return False
            return match_tokens(tokens, p_idx + len(val), t_idx + 1)
        
        elif token[0] == 'escape':
            _, char = token
            if p_idx >= len(path) or path[p_idx] != char:
                return False
            return match_tokens(tokens, p_idx + 1, t_idx + 1)
        
        elif token[0] == 'star':
            for i in range(p_idx, len(path) + 1):
                if match_tokens(tokens, i, t_idx + 1):
                    return True
            return False
        
        elif token[0] == 'double_star':
            if p_idx > len(path):
                return False
            for i in range(p_idx, len(path) + 1):
                if match_tokens(tokens, i, t_idx + 1):
                    return True
            return False
        
        elif token[0] == 'question':
            if p_idx >= len(path) or path[p_idx] == '/':
                return False
            return match_tokens(tokens, p_idx + 1, t_idx + 1)
        
        elif token[0] == 'char_class':
            if not match_char_class(tokens, p_idx, token[1], token[2]):
                return False
            return match_tokens(tokens, p_idx + 1, t_idx + 1)
        
        return False
    
    tokens = tokenize(pattern)
    return match_tokens(tokens, 0, 0)