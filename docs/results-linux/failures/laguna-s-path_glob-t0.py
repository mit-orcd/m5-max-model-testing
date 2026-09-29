def match(pattern, path):
    def parse_char_class(s, i):
        # s[i] == '['
        i += 1
        negate = False
        if i < len(s) and s[i] == '!':
            negate = True
            i += 1
        chars = set()
        while i < len(s) and s[i] != ']':
            if s[i] == '\\' and i + 1 < len(s):
                chars.add(s[i + 1])
                i += 2
            else:
                chars.add(s[i])
                i += 1
        i += 1  # skip ']'
        return chars, negate, i

    def match_here(p, t):
        while True:
            if not p:
                return not t
            c = p[0]
            if c == '\\':
                if len(p) < 2 or not t:
                    return False
                if p[1] != t[0]:
                    return False
                p = p[2:]
                t = t[1:]
            elif c == '?':
                if not t or t[0] == '/':
                    return False
                p = p[1:]
                t = t[1:]
            elif c == '*':
                if len(p) > 1 and p[1] == '*':
                    # ** segment
                    if len(p) > 2 and p[2] == '/':
                        # **/ -> match zero or more segments
                        rest = p[3:]
                        if not rest:
                            return True
                        # try matching rest at current position and after each '/'
                        if match_here(rest, t):
                            return True
                        while True:
                            idx = t.find('/')
                            if idx == -1:
                                return False
                            t = t[idx + 1:]
                            if match_here(rest, t):
                                return True
                    else:
                        # ** not followed by / -> treat as *
                        p = p[1:]
                        continue
                else:
                    # single * -> match non-slash chars
                    p = p[1:]
                    while t and t[0] != '/':
                        if match_here(p, t):
                            return True
                        t = t[1:]
                    return match_here(p, t)
            elif c == '[':
                if not t or t[0] == '/':
                    return False
                chars, negate, new_i = parse_char_class(p, 0)
                p = p[new_i:]
                if (t[0] in chars) == negate:
                    return False
                t = t[1:]
            elif c == '/':
                if not t or t[0] != '/':
                    return False
                p = p[1:]
                t = t[1:]
            else:
                if not t or t[0] != c:
                    return False
                p = p[1:]
                t = t[1:]

    return match_here(pattern, path)