def match(pattern, path):
    def _compile(pat):
        tokens = []
        i = 0
        while i < len(pat):
            c = pat[i]
            if c == '\\':
                i += 1
                tokens.append(('literal', pat[i]))
            elif c == '?':
                tokens.append(('any_char', None))
            elif c == '*':
                if i + 1 < len(pat) and pat[i + 1] == '*':
                    tokens.append(('star_star', None))
                    i += 2
                else:
                    tokens.append(('star', None))
                    i += 1
            elif c == '[':
                j = i + 1
                negated = False
                chars = set()
                if j < len(pat) and pat[j] == '!':
                    negated = True
                    j += 1
                while j < len(pat) and pat[j] != ']':
                    if j + 2 < len(pat) and pat[j + 1] == '-' and pat[j + 2] != ']':
                        start, end = pat[j], pat[j + 2]
                        for k in range(ord(start), ord(end) + 1):
                            chars.add(chr(k))
                        j += 3
                    else:
                        chars.add(pat[j])
                        j += 1
                tokens.append(('class', (chars, negated)))
                i = j + 1
            else:
                tokens.append(('literal', c))
                i += 1
        return tokens

    def _match_tokens(tokens, ti, path):
        while ti < len(tokens):
            tag, val = tokens[ti]
            if tag == 'literal':
                if not path or path[0] != val:
                    return False
                path = path[1:]
                ti += 1
            elif tag == 'any_char':
                if not path or path[0] == '/':
                    return False
                path = path[1:]
                ti += 1
            elif tag == 'star':
                # Match zero or more non-/ characters
                for k in range(len(path)):
                    if path[k] == '/':
                        break
                    if _match_tokens(tokens, ti + 1, path[k + 1:]):
                        return True
                return _match_tokens(tokens, ti + 1, path)
            elif tag == 'star_star':
                # Match zero or more whole segments
                if _match_tokens(tokens, ti + 1, path):
                    return True
                # Skip past current segment and match rest
                k = 0
                while k < len(path) and path[k] != '/':
                    k += 1
                # Try after each '/'
                while k < len(path):
                    if path[k] == '/':
                        if _match_tokens(tokens, ti + 1, path[k + 1:]):
                            return True
                    k += 1
                return False
            elif tag == 'class':
                chars, negated = val
                if not path or path[0] == '/':
                    return False
                in_class = path[0] in chars
                if negated:
                    in_class = not in_class
                if not in_class:
                    return False
                path = path[1:]
                ti += 1
        return path == ''

    tokens = _compile(pattern)
    return _match_tokens(tokens, 0, path)