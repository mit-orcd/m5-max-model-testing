def match(pattern, path):
    def parse_pattern(p):
        i = 0
        tokens = []
        while i < len(p):
            if p[i] == '\\':
                i += 1
                if i < len(p):
                    tokens.append(('LIT', p[i]))
                else:
                    tokens.append(('LIT', '\\'))
                i += 1
            elif p[i] == '*':
                if i + 1 < len(p) and p[i+1] == '*':
                    tokens.append(('DBLSTAR', None))
                    i += 2
                else:
                    tokens.append(('STAR', None))
                    i += 1
            elif p[i] == '?':
                tokens.append(('QMARK', None))
                i += 1
            elif p[i] == '[':
                j = i + 1
                negate = False
                if j < len(p) and p[j] == '!':
                    negate = True
                    j += 1
                chars = []
                while j < len(p) and p[j] != ']':
                    if j + 2 < len(p) and p[j+1] == '-':
                        start = p[j]
                        end = p[j+2]
                        if start <= end:
                            chars.extend(chr(c) for c in range(ord(start), ord(end)+1))
                        else:
                            chars.append(start)
                            chars.append('-')
                            chars.append(end)
                        j += 3
                    else:
                        chars.append(p[j])
                        j += 1
                if j < len(p) and p[j] == ']':
                    j += 1
                else:
                    chars.append('[')
                    if negate:
                        chars.insert(0, '!')
                    chars = ['['] + chars
                tokens.append(('CLASS', (set(chars), negate)))
                i = j
            else:
                tokens.append(('LIT', p[i]))
                i += 1
        return tokens

    def match_class(c, class_set, negate):
        in_set = c in class_set
        return not in_set if negate else in_set

    def match_token(tok, char):
        typ, val = tok
        if typ == 'LIT':
            return char == val
        elif typ == 'QMARK':
            return char != '/'
        elif typ == 'STAR':
            return char != '/'
        elif typ == 'CLASS':
            return match_token(('LIT', char), char) and match_class(char, val[0], val[1])
        return False

    tokens = parse_pattern(pattern)
    path_parts = path.split('/')
    n = len(tokens)
    m = len(path_parts)

    from functools import lru_cache

    @lru_cache(maxsize=None)
    def dp(i, j):
        if i == n and j == m:
            return True
        if i == n:
            return False
        if j > m:
            return False

        tok = tokens[i]
        typ, val = tok

        if typ == 'DBLSTAR':
            # Match zero or more segments
            # Option 1: match zero segments -> move to next token
            if dp(i+1, j):
                return True
            # Option 2: match one or more segments -> consume one segment, stay on DBLSTAR
            if j < m:
                # Check if current segment matches the pattern after DBLSTAR? No, DBLSTAR matches whole segments arbitrarily
                # So we just consume one segment and stay on DBLSTAR
                return dp(i, j+1)
            return False

        elif typ in ('LIT', 'QMARK', 'STAR', 'CLASS'):
            if j >= m:
                return False
            segment = path_parts[j]
            if typ == 'LIT':
                return segment == val and dp(i+1, j+1)
            elif typ == 'QMARK':
                return len(segment) == 1 and segment != '/' and dp(i+1, j+1)
            elif typ == 'STAR':
                # Match zero or more chars in segment, not crossing '/'
                k = 0
                while k <= len(segment):
                    if match_token(tok, segment[k]) if k < len(segment) else True:
                        # If we've matched up to k, and rest of segment is matched by STAR (zero or more)
                        # Actually, STAR matches zero or more non-/ chars in the segment
                        # So we need to match the whole segment with STAR*
                        # But STAR is greedy: we try to match as much as possible, but we need to match the whole segment
                        # Better: STAR matches zero or more non-/ chars, so we can match any prefix of the segment
                        # Then the rest must be matched by nothing? No, STAR consumes the whole segment if it's the only thing
                        # Actually, for a single segment, STAR must match the entire segment (since no '/' inside)
                        # So we need to check if the entire segment can be matched by STAR*
                        # But STAR matches zero or more non-/ chars -> so any string without '/' is matched by STAR*
                        # Since segment has no '/' (split by '/'), STAR always matches the whole segment
                        # So we just need to move to next token and next segment
                        return dp(i+1, j+1)
                    k += 1
                return False
            elif typ == 'CLASS':
                return len(segment) == 1 and segment != '/' and match_class(segment, val[0], val[1]) and dp(i+1, j+1)
        return False

    return dp(0, 0)