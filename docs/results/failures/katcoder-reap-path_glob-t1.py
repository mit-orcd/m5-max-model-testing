def match(pattern, path):
    def compile_pattern(pattern):
        tokens = []
        i = 0
        while i < len(pattern):
            c = pattern[i]
            if c == '\\':
                i += 1
                tokens.append(('literal', pattern[i]))
            elif c == '?':
                tokens.append(('question',))
            elif c == '*':
                if i + 1 < len(pattern) and pattern[i + 1] == '*':
                    tokens.append(('starstar',))
                    i += 2
                else:
                    tokens.append(('star',))
                    i += 1
            elif c == '[':
                j = i + 1
                neg = False
                chars = []
                if j < len(pattern) and pattern[j] == '!':
                    neg = True
                    j += 1
                while j < len(pattern) and pattern[j] != ']':
                    if pattern[j] == '\\' and j + 1 < len(pattern):
                        chars.append(pattern[j + 1])
                        j += 2
                    elif j + 2 < len(pattern) and pattern[j + 1] == '-' and pattern[j + 2] != ']':
                        start = pattern[j]
                        end = pattern[j + 2]
                        for ch in range(ord(start), ord(end) + 1):
                            chars.append(chr(ch))
                        j += 3
                    else:
                        chars.append(pattern[j])
                        j += 1
                tokens.append(('class', set(chars), neg))
                i = j + 1
            else:
                tokens.append(('literal', c))
                i += 1
        return tokens

    def match_tokens(tokens, path):
        ti = 0
        pi = 0
        path_segments = path.split('/') if path else []
        # Rebuild as we need to handle ** which can match zero or more segments
        # Actually, let's work with the raw path string and indices
        return match_recursive(tokens, 0, path, 0)

    def match_recursive(tokens, ti, path, pi):
        while ti < len(tokens):
            token = tokens[ti]
            if token[0] == 'literal':
                if pi >= len(path) or path[pi] != token[1]:
                    return False
                pi += 1
                ti += 1
            elif token[0] == 'question':
                if pi >= len(path) or path[pi] == '/':
                    return False
                pi += 1
                ti += 1
            elif token[0] == 'star':
                # Match zero or more non-/ characters
                ti += 1
                # Try matching 0, 1, 2, ... characters
                for k in range(pi, len(path) + 1):
                    if k > pi and path[k - 1] == '/':
                        break
                    if match_recursive(tokens, ti, path, k):
                        return True
                return False
            elif token[0] == 'starstar':
                ti += 1
                # ** matches zero or more whole segments
                # Try matching at each segment boundary
                for k in range(pi, len(path) + 1):
                    if k > pi and path[k - 1] != '/':
                        continue
                    if match_recursive(tokens, ti, path, k):
                        return True
                return False
            elif token[0] == 'class':
                if pi >= len(path) or path[pi] == '/':
                    return False
                ch = path[pi]
                chars, neg = token[1], token[2]
                if neg:
                    if ch in chars:
                        return False
                else:
                    if ch not in chars:
                        return False
                pi += 1
                ti += 1
        return pi == len(path)

    tokens = compile_pattern(pattern)
    return match_recursive(tokens, 0, path, 0)