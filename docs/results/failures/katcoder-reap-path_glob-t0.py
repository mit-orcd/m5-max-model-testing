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
                tokens.append(('any',))
            elif c == '*':
                if i + 1 < len(pat) and pat[i + 1] == '*':
                    tokens.append(('starstar',))
                    i += 2
                else:
                    tokens.append(('star',))
                    i += 1
            elif c == '[':
                j = i + 1
                neg = False
                chars = []
                if j < len(pat) and pat[j] == '!':
                    neg = True
                    j += 1
                while j < len(pat) and pat[j] != ']':
                    if pat[j] == '\\' and j + 1 < len(pat):
                        chars.append(pat[j + 1])
                        j += 2
                    elif j + 2 < len(pat) and pat[j + 1] == '-' and pat[j + 2] != ']':
                        start = pat[j]
                        end = pat[j + 2]
                        for k in range(ord(start), ord(end) + 1):
                            chars.append(chr(k))
                        j += 3
                    else:
                        chars.append(pat[j])
                        j += 1
                j += 1
                tokens.append(('class', chars, neg))
                i = j
            else:
                tokens.append(('literal', c))
                i += 1
        return tokens

    def _match_tokens(tokens, ti, path, pi):
        while ti < len(tokens):
            tok = tokens[ti]
            if tok[0] == 'literal':
                if pi >= len(path) or path[pi] != tok[1]:
                    return False
                ti += 1
                pi += 1
            elif tok[0] == 'any':
                if pi >= len(path) or path[pi] == '/':
                    return False
                ti += 1
                pi += 1
            elif tok[0] == 'class':
                if pi >= len(path) or path[pi] == '/':
                    return False
                ch = path[pi]
                matched = ch in tok[1]
                if tok[2]:
                    matched = not matched
                if not matched:
                    return False
                ti += 1
                pi += 1
            elif tok[0] == 'star':
                # match zero or more non-/ characters
                ti += 1
                for k in range(pi, len(path) + 1):
                    if k > pi and path[k - 1] == '/':
                        break
                    if _match_tokens(tokens, ti, path, k):
                        return True
                return False
            elif tok[0] == 'starstar':
                # match zero or more whole segments
                ti += 1
                # try matching at current position, and after each '/'
                for k in range(pi, len(path) + 1):
                    if k > pi and path[k - 1] == '/':
                        if _match_tokens(tokens, ti, path, k):
                            return True
                    elif k == len(path):
                        if _match_tokens(tokens, ti, path, k):
                            return True
                return False
        return pi == len(path)

    tokens = _compile(pattern)
    return _match_tokens(tokens, 0, path, 0)