def match(pattern, path):
    # Tokenize pattern into a list of tokens
    tokens = []
    i = 0
    n = len(pattern)
    while i < n:
        c = pattern[i]
        if c == '\\':
            if i + 1 < n:
                tokens.append(('LIT', pattern[i+1]))
                i += 2
            else:
                tokens.append(('LIT', '\\'))
                i += 1
        elif c == '?':
            tokens.append(('QMARK',))
            i += 1
        elif c == '*':
            if i + 1 < n and pattern[i+1] == '*':
                tokens.append(('STAR',))
                i += 2
            else:
                tokens.append(('SINGLESTAR',))
                i += 1
        elif c == '[':
            j = i + 1
            negated = False
            if j < n and pattern[j] == '!':
                negated = True
                j += 1
            if j >= n:
                tokens.append(('LIT', '['))
                i += 1
                continue
            chars = set()
            while j < n and pattern[j] != ']':
                if pattern[j] == '\\' and j + 1 < n:
                    chars.add(pattern[j+1])
                    j += 2
                elif j + 2 < n and pattern[j+1] == '-' and pattern[j+2] != ']':
                    start, end = pattern[j], pattern[j+2]
                    if start <= end:
                        for k in range(ord(start), ord(end) + 1):
                            chars.add(chr(k))
                    j += 3
                else:
                    chars.add(pattern[j])
                    j += 1
            if j < n and pattern[j] == ']':
                tokens.append(('CLASS', chars, negated))
                i = j + 1
            else:
                tokens.append(('LIT', '['))
                i += 1
        else:
            tokens.append(('LIT', c))
            i += 1

    # Split path into segments
    path_segments = path.split('/') if path else []
    if path == '':
        path_segments = []
    elif path == '/':
        path_segments = ['']
    else:
        path_segments = path.split('/')

    # Recursive matching with memoization
    from functools import lru_cache

    @lru_cache(maxsize=None)
    def match_tokens(t_idx, p_idx):
        # If we've consumed all tokens
        if t_idx == len(tokens):
            return p_idx == len(path_segments)

        token = tokens[t_idx]
        kind = token[0]

        if kind == 'LIT':
            if p_idx >= len(path_segments):
                return False
            seg = path_segments[p_idx]
            if seg == token[1]:
                return match_tokens(t_idx + 1, p_idx + 1)
            return False

        elif kind == 'QMARK':
            if p_idx >= len(path_segments):
                return False
            seg = path_segments[p_idx]
            if len(seg) == 1 and seg != '/':
                return match_tokens(t_idx + 1, p_idx + 1)
            return False

        elif kind == 'SINGLESTAR':
            if p_idx >= len(path_segments):
                return False
            seg = path_segments[p_idx]
            if '/' not in seg:
                return match_tokens(t_idx + 1, p_idx + 1)
            return False

        elif kind == 'STAR':
            # ** matches zero or more whole segments
            # Try matching zero segments
            if match_tokens(t_idx + 1, p_idx):
                return True
            # Try matching one or more segments
            for k in range(p_idx, len(path_segments)):
                if match_tokens(t_idx + 1, k + 1):
                    return True
            return False

        elif kind == 'CLASS':
            if p_idx >= len(path_segments):
                return False
            seg = path_segments[p_idx]
            if len(seg) != 1:
                return False
            char = seg
            if char == '/':
                return False
            chars, negated = token[1], token[2]
            in_class = char in chars
            if negated:
                in_class = not in_class
            if in_class:
                return match_tokens(t_idx + 1, p_idx + 1)
            return False

        return False

    return match_tokens(0, 0)