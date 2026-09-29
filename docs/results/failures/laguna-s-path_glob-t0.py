def match(pattern, path):
    def parse_pattern(p):
        tokens = []
        i = 0
        n = len(p)
        while i < n:
            c = p[i]
            if c == '\\':
                if i + 1 < n:
                    tokens.append(('literal', p[i+1]))
                    i += 2
                else:
                    tokens.append(('literal', '\\'))
                    i += 1
            elif c == '[':
                j = i + 1
                negate = False
                if j < n and p[j] == '!':
                    negate = True
                    j += 1
                chars = []
                while j < n and p[j] != ']':
                    if p[j] == '\\' and j + 1 < n:
                        chars.append(p[j+1])
                        j += 2
                    else:
                        chars.append(p[j])
                        j += 1
                if j >= n:
                    tokens.append(('literal', '['))
                    i += 1
                else:
                    tokens.append(('class', (chars, negate)))
                    i = j + 1
            elif c == '*':
                if i + 1 < n and p[i+1] == '*':
                    if i + 2 < n and p[i+2] == '/':
                        tokens.append(('starstar', None))
                        i += 3
                    else:
                        tokens.append(('starstar', None))
                        i += 2
                else:
                    tokens.append(('star', None))
                    i += 1
            elif c == '?':
                tokens.append(('question', None))
                i += 1
            else:
                tokens.append(('literal', c))
                i += 1
        return tokens

    def match_here(tokens, ti, path, pi):
        while ti < len(tokens):
            token = tokens[ti]
            if token[0] == 'literal':
                if pi < len(path) and path[pi] == token[1]:
                    pi += 1
                    ti += 1
                else:
                    return False
            elif token[0] == 'question':
                if pi < len(path) and path[pi] != '/':
                    pi += 1
                    ti += 1
                else:
                    return False
            elif token[0] == 'star':
                if ti + 1 < len(tokens):
                    next_token = tokens[ti + 1]
                    if next_token[0] == 'literal':
                        target = next_token[1]
                        while pi < len(path) and path[pi] != '/' and path[pi] != target:
                            pi += 1
                        if pi < len(path) and path[pi] == target:
                            if match_here(tokens, ti + 2, path, pi + 1):
                                return True
                            pi += 1
                            while pi < len(path) and path[pi] != '/' and path[pi] != target:
                                pi += 1
                            if pi < len(path) and path[pi] == target:
                                if match_here(tokens, ti + 2, path, pi + 1):
                                    return True
                            return False
                        else:
                            return False
                    elif next_token[0] == 'class':
                        chars, negate = next_token[1]
                        while pi < len(path) and path[pi] != '/':
                            if (path[pi] in chars) != (not negate):
                                if match_here(tokens, ti + 2, path, pi + 1):
                                    return True
                            pi += 1
                        return False
                    elif next_token[0] == 'question':
                        while pi < len(path) and path[pi] != '/':
                            if match_here(tokens, ti + 2, path, pi + 1):
                                return True
                            pi += 1
                        return False
                    elif next_token[0] == 'star':
                        return match_here(tokens, ti + 1, path, pi)
                    elif next_token[0] == 'starstar':
                        return match_here(tokens, ti + 1, path, pi)
                    else:
                        return False
                else:
                    while pi < len(path) and path[pi] != '/':
                        pi += 1
                    return pi == len(path)
            elif token[0] == 'starstar':
                if ti + 1 >= len(tokens):
                    return True
                next_token = tokens[ti + 1]
                if next_token[0] == 'literal':
                    target = next_token[1]
                    while True:
                        if pi < len(path) and path[pi] == target:
                            if match_here(tokens, ti + 2, path, pi + 1):
                                return True
                        if pi >= len(path) or path[pi] == '/':
                            break
                        pi += 1
                    return False
                elif next_token[0] == 'class':
                    chars, negate = next_token[1]
                    while True:
                        if pi < len(path) and (path[pi] in chars) != (not negate):
                            if match_here(tokens, ti + 2, path, pi + 1):
                                return True
                        if pi >= len(path) or path[pi] == '/':
                            break
                        pi += 1
                    return False
                elif next_token[0] == 'question':
                    while True:
                        if pi < len(path) and path[pi] != '/':
                            if match_here(tokens, ti + 2, path, pi + 1):
                                return True
                        if pi >= len(path) or path[pi] == '/':
                            break
                        pi += 1
                    return False
                elif next_token[0] == 'star':
                    return match_here(tokens, ti + 1, path, pi)
                elif next_token[0] == 'starstar':
                    return match_here(tokens, ti + 1, path, pi)
                else:
                    return False
            else:
                return False
        return pi == len(path)

    tokens = parse_pattern(pattern)
    return match_here(tokens, 0, path, 0)