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
            if s[i] == '-' and i + 2 < len(s) and s[i+2] != ']':
                start, end = s[i-1], s[i+1]
                for c in range(ord(start) + 1, ord(end)):
                    chars.add(chr(c))
                i += 2
            else:
                chars.add(s[i])
            i += 1
        if i >= len(s):
            return None, 0
        if negated:
            chars = set(chr(c) for c in range(256)) - chars
        return chars, i + 1

    def parse_pattern(s, i):
        tokens = []
        while i < len(s):
            if s[i] == '\\' and i + 1 < len(s):
                tokens.append(('literal', s[i+1]))
                i += 2
            elif s[i] == '?':
                tokens.append(('any_single',))
                i += 1
            elif s[i] == '*':
                tokens.append(('any_zero_or_more',))
                i += 1
            elif s[i] == '[':
                chars, i = parse_char_class(s, i)
                if chars is None:
                    return None
                tokens.append(('char_class', chars))
            elif s[i] == '/':
                tokens.append(('slash',))
                i += 1
            else:
                tokens.append(('literal', s[i]))
                i += 1
        return tokens

    def match_tokens(tokens, path, p):
        if not tokens:
            return p == len(path)
        token = tokens[0]
        rest = tokens[1:]
        if token[0] == 'literal':
            if p < len(path) and path[p] == token[1]:
                return match_tokens(rest, path, p + 1)
            return False
        elif token[0] == 'slash':
            if p < len(path) and path[p] == '/':
                return match_tokens(rest, path, p + 1)
            return False
        elif token[0] == 'any_single':
            if p < len(path) and path[p] != '/':
                return match_tokens(rest, path, p + 1)
            return False
        elif token[0] == 'any_zero_or_more':
            if match_tokens(rest, path, p):
                return True
            if p < len(path) and path[p] != '/':
                return match_tokens(tokens, path, p + 1)
            return False
        elif token[0] == 'char_class':
            if p < len(path) and path[p] in token[1]:
                return match_tokens(rest, path, p + 1)
            return False
        return False

    pattern_tokens = parse_pattern(pattern, 0)
    if pattern_tokens is None:
        return False
    return match_tokens(pattern_tokens, path, 0)