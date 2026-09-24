def match(pattern, path):
    def parse_char_class(s, i):
        negated = False
        chars = set()
        if i < len(s) and s[i] == '!':
            negated = True
            i += 1
        if i < len(s) and s[i] == ']':
            chars.add(']')
            i += 1
        while i < len(s) and s[i] != ']':
            if s[i] == '\\' and i + 1 < len(s):
                chars.add(s[i+1])
                i += 2
            else:
                chars.add(s[i])
                i += 1
        if i < len(s) and s[i] == ']':
            i += 1
        return chars, negated, i

    def matches_char(c, chars, negated):
        return (c in chars) != negated

    def do_match(p, i, s, j):
        while i < len(p):
            c = p[i]
            if c == '\\':
                if i + 1 >= len(p) or j >= len(s):
                    return False
                if p[i+1] != s[j]:
                    return False
                i += 2
                j += 1
            elif c == '?':
                if j >= len(s) or s[j] == '/':
                    return False
                i += 1
                j += 1
            elif c == '*':
                if i + 1 < len(p) and p[i+1] == '*':
                    # Check for **/ pattern
                    if i + 2 < len(p) and p[i+2] == '/':
                        # **/ matches zero or more segments
                        # Try matching zero segments first
                        if do_match(p, i + 3, s, j):
                            return True
                        # Try matching one or more segments
                        while j < len(s):
                            if s[j] == '/':
                                if do_match(p, i + 3, s, j + 1):
                                    return True
                            j += 1
                        return False
                    elif i + 2 == len(p):
                        # ** at end of pattern matches rest of string
                        return True
                    else:
                        # ** not followed by / - treat as single *
                        k = j
                        while k < len(s) and s[k] != '/':
                            k += 1
                        while j <= k:
                            if do_match(p, i + 1, s, j):
                                return True
                            j += 1
                        return False
                else:
                    # Single * matches within a segment
                    k = j
                    while k < len(s) and s[k] != '/':
                        k += 1
                    while j <= k:
                        if do_match(p, i + 1, s, j):
                            return True
                        j += 1
                    return False
            elif c == '[':
                if j >= len(s) or s[j] == '/':
                    return False
                chars, negated, ni = parse_char_class(p, i + 1)
                if not matches_char(s[j], chars, negated):
                    return False
                i = ni
                j += 1
            elif c == '/':
                if j >= len(s) or s[j] != '/':
                    return False
                i += 1
                j += 1
            else:
                if j >= len(s) or s[j] != c:
                    return False
                i += 1
                j += 1
        return j == len(s)

    return do_match(pattern, 0, path, 0)