def match(pattern, path):
    def parse_char_class(s, i):
        negated = False
        if i < len(s) and s[i] == '!':
            negated = True
            i += 1
        chars = set()
        while i < len(s) and s[i] != ']':
            if s[i] == '-' and i > 0 and i < len(s) - 1:
                start = s[i-1]
                end = s[i+1]
                for code in range(ord(start), ord(end) + 1):
                    chars.add(chr(code))
                i += 2
            elif s[i] == '\\' and i + 1 < len(s):
                chars.add(s[i+1])
                i += 2
            else:
                chars.add(s[i])
                i += 1
        return chars, negated, i + 1

    def parse_pattern(s):
        tokens = []
        i = 0
        while i < len(s):
            if s[i] == '\\' and i + 1 < len(s):
                tokens.append(('literal', s[i+1]))
                i += 2
            elif s[i] == '?':
                tokens.append(('any_single',))
                i += 1
            elif s[i] == '*':
                if i + 1 < len(s) and s[i+1] == '*':
                    tokens.append(('star_star',))
                    i += 2
                else:
                    tokens.append(('star',))
                    i += 1
            elif s[i] == '[':
                chars, negated, i = parse_char_class(s, i + 1)
                tokens.append(('char_class', chars, negated))
            else:
                tokens.append(('literal', s[i]))
                i += 1
        return tokens

    def match_tokens(tokens, path, i, j):
        if not tokens:
            return j == len(path)
        
        token = tokens[0]
        rest = tokens[1:]
        
        if token == ('star',):
            while j <= len(path):
                if match_tokens(rest, path, i, j):
                    return True
                j += 1
            return False
        
        elif token == ('star_star',):
            if not rest:
                return j == len(path)
            while True:
                if match_tokens(rest, path, i, j):
                    return True
                if j >= len(path):
                    return False
                j += 1
        
        elif token == ('any_single',):
            if j < len(path) and path[j] != '/':
                return match_tokens(rest, path, i, j + 1)
            return False
        
        elif token[0] == 'char_class':
            if j < len(path):
                chars, negated = token[1], token[2]
                c = path[j]
                matched = (c in chars) != negated
                if matched:
                    return match_tokens(rest, path, i, j + 1)
            return False
        
        elif token == ('literal',):
            if j < len(path) and path[j] == token[1]:
                return match_tokens(rest, path, i, j + 1)
            return False
        
        return False

    tokens = parse_pattern(pattern)
    return match_tokens(tokens, path, 0, 0)