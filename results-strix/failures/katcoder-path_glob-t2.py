def match(pattern, path):
    def parse_pattern(pat):
        tokens = []
        i = 0
        while i < len(pat):
            c = pat[i]
            if c == '\\':
                i += 1
                tokens.append(('literal', pat[i]))
            elif c == '?':
                tokens.append(('question',))
            elif c == '*':
                j = i
                while j < len(pat) and pat[j] == '*':
                    j += 1
                if j - i >= 2:
                    # Check if it's ** or **/
                    rest = pat[j:]
                    if rest.startswith('/'):
                        tokens.append(('starstar',))
                        i = j + 1
                    else:
                        tokens.append(('starstar',))
                        i = j
                else:
                    tokens.append(('star',))
                    i = j
            elif c == '[':
                j = i + 1
                negated = False
                if j < len(pat) and pat[j] == '!':
                    negated = True
                    j += 1
                chars = set()
                while j < len(pat) and pat[j] != ']':
                    if j + 2 < len(pat) and pat[j + 1] == '-' and pat[j + 2] != ']':
                        start = pat[j]
                        end = pat[j + 2]
                        for code in range(ord(start), ord(end) + 1):
                            chars.add(chr(code))
                        j += 3
                    else:
                        chars.add(pat[j])
                        j += 1
                tokens.append(('class', chars, negated))
                i = j + 1
            else:
                tokens.append(('literal', c))
                i += 1
        return tokens

    def match_tokens(pat_tokens, path_segments):
        pi = 0
        si = 0
        return _match(pi, si, pat_tokens, path_segments)

    def _match(pi, si, pat_tokens, path_segments):
        while pi < len(pat_tokens) and si < len(path_segments):
            tok = pat_tokens[pi]
            if tok[0] == 'literal':
                if path_segments[si] == tok[1]:
                    pi += 1
                    si += 1
                else:
                    return False
            elif tok[0] == 'question':
                if path_segments[si] == '/':
                    return False
                pi += 1
                si += 1
            elif tok[0] == 'star':
                # Try matching zero or more characters within the segment
                rest = pat_tokens[pi + 1:]
                for end in range(si, len(path_segments) + 1):
                    if _match(pi + 1, end, pat_tokens, path_segments):
                        return True
                return False
            elif tok[0] == 'starstar':
                # ** matches zero or more whole segments
                rest = pat_tokens[pi + 1:]
                for end in range(si, len(path_segments) + 1):
                    if _match(pi + 1, end, pat_tokens, path_segments):
                        return True
                return False
            elif tok[0] == 'class':
                chars, negated = tok[1], tok[2]
                ch = path_segments[si]
                if ch == '/':
                    return False
                if negated:
                    if ch not in chars:
                        pi += 1
                        si += 1
                    else:
                        return False
                else:
                    if ch in chars:
                        pi += 1
                        si += 1
                    else:
                        return False
        # Consume remaining ** patterns
        while pi < len(pat_tokens) and pat_tokens[pi][0] == 'starstar':
            pi += 1
        return pi == len(pat_tokens) and si == len(path_segments)

    pat_tokens = parse_pattern(pattern)
    path_segments = path.split('/')
    return match_tokens(pat_tokens, path_segments)