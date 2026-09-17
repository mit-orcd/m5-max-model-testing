def match(pattern, path):
    def compile_pattern(p):
        tokens = []
        i = 0
        n = len(p)
        while i < n:
            if p[i] == '\\':
                if i + 1 < n:
                    tokens.append(('literal', p[i+1]))
                    i += 2
                    continue
                else:
                    tokens.append(('literal', '\\'))
                    i += 1
            elif p[i] == '*':
                if i + 1 < n and p[i+1] == '*':
                    tokens.append(('double_star',))
                    i += 2
                else:
                    tokens.append(('star',))
                    i += 1
            elif p[i] == '?':
                tokens.append(('question',))
                i += 1
            elif p[i] == '[':
                j = i + 1
                negate = False
                if j < n and p[j] == '!':
                    negate = True
                    j += 1
                chars = []
                while j < n and p[j] != ']':
                    if j + 1 < n and p[j+1] == '-':
                        if j + 2 >= n or p[j+2] == ']':
                            chars.append(p[j])
                            j += 1
                            continue
                        chars.append((p[j], p[j+2]))
                        j += 3
                    else:
                        chars.append(p[j])
                        j += 1
                if j >= n:
                    return None
                tokens.append(('class', chars, negate))
                i = j + 1
            elif p[i] == '/':
                tokens.append(('literal', '/'))
                i += 1
            else:
                tokens.append(('literal', p[i]))
                i += 1
        return tokens

    def match_segment(tokens, s):
        i = 0
        n = len(tokens)
        j = 0
        m = len(s)
        while i < n and j < m:
            if tokens[i][0] == 'literal':
                if tokens[i][1] != s[j]:
                    return False
                i += 1
                j += 1
            elif tokens[i][0] == 'question':
                i += 1
                j += 1
            elif tokens[i][0] == 'class':
                chars, negate = tokens[i][1], tokens[i][2]
                matched = False
                for c in chars:
                    if isinstance(c, tuple):
                        if s[j] >= c[0] and s[j] <= c[1]:
                            matched = True
                            break
                    else:
                        if s[j] == c:
                            matched = True
                            break
                if matched == negate:
                    return False
                i += 1
                j += 1
            else:
                return False
        return i == n and j == m

    def match_path(tokens, segments):
        i = 0
        n = len(tokens)
        j = 0
        m = len(segments)
        while i < n and j < m:
            if tokens[i][0] == 'literal' and tokens[i][1] == '/':
                if not match_segment(tokens[i:i+1], ''):
                    return False
                i += 1
            elif tokens[i][0] == 'double_star':
                i += 1
                while i < n and tokens[i][0] == 'literal' and tokens[i][1] == '/':
                    i += 1
                while j < m:
                    if match_segment(tokens[i:], segments[j]):
                        j += 1
                        break
                    j += 1
                if j >= m:
                    return False
            else:
                if not match_segment(tokens[i:i+1], segments[j]):
                    return False
                i += 1
                j += 1
        while i < n and tokens[i][0] == 'literal' and tokens[i][1] == '/':
            i += 1
        return i == n and j == m

    tokens = compile_pattern(pattern)
    if tokens is None:
        return False
    segments = path.split('/')
    return match_path(tokens, segments)