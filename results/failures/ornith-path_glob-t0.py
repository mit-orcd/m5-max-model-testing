def match(pattern, path):
    def parse_class(s, i):
        # Returns (negated, chars_set, next_index) or None if invalid
        negated = False
        i += 1  # skip '['
        if i < len(s) and s[i] == '!':
            negated = True
            i += 1
        chars = set()
        while i < len(s) and s[i] != ']':
            if s[i] == '\\' and i + 1 < len(s):
                chars.add(s[i + 1])
                i += 2
            else:
                if i + 2 < len(s) and s[i + 1] == '-' and s[i + 2] != ']':
                    lo = s[i]
                    hi = s[i + 2]
                    for c in range(ord(lo), ord(hi) + 1):
                        chars.add(chr(c))
                    i += 3
                else:
                    chars.add(s[i])
                    i += 1
        if i >= len(s):
            return None
        return (negated, chars, i + 1)

    def match_here(p, pi, path, si):
        while pi < len(p):
            c = p[pi]
            if c == '*':
                # skip consecutive stars
                while pi < len(p) and p[pi] == '*':
                    pi += 1
                if pi < len(p) and p[pi] == '/':
                    pi += 1
                    doublestar = True
                else:
                    doublestar = False
                # try to match rest starting at each position
                if not doublestar:
                    # match rest without crossing '/'
                    for k in range(si, len(path) + 1):
                        if path[k] == '/' and k > si:
                            break
                        if match_here(p, pi, path, k):
                            return True
                    return False
                else:
                    # double star: match zero or more segments
                    for k in range(si, len(path) + 1):
                        if match_here(p, pi, path, k):
                            return True
                        if k < len(path) and path[k] == '/':
                            continue
                        # can't cross into next segment boundary check
                        if k < len(path):
                            continue
                    return False
            elif c == '?':
                if si >= len(path) or path[si] == '/' or path[si] == '\\':
                    return False
                si += 1
                pi += 1
            elif c == '[':
                res = parse_class(p, pi)
                if res is None:
                    # treat '[' literally
                    if si >= len(path) or path[si] != '[':
                        return False
                    si += 1
                    pi += 1
                else:
                    negated, chars, next_i = res
                    if si >= len(path) or path[si] == '/' or path[si] == '\\':
                        return False
                    ch = path[si]
                    in_class = ch in chars
                    if negated:
                        if in_class:
                            return False
                    else:
                        if not in_class:
                            return False
                    si += 1
                    pi = next_i
            elif c == '\\':
                if pi + 1 < len(p):
                    esc = p[pi + 1]
                    if si >= len(path) or path[si] != esc:
                        return False
                    si += 1
                    pi += 2
                else:
                    if si >= len(path) or path[si] != '\\':
                        return False
                    si += 1
                    pi += 1
            else:
                if si >= len(path) or path[si] != c:
                    return False
                si += 1
                pi += 1
        return si == len(path)

    return match_here(pattern, 0, path, 0)