def match(pattern, path):
    def parse_class(pattern, pos):
        i = pos + 1
        negated = False
        if i < len(pattern) and pattern[i] == '!':
            negated = True
            i += 1
        chars = set()
        while i < len(pattern) and (pattern[i] != ']' or i == pos + 1):
            if pattern[i] == '\\' and i + 1 < len(pattern):
                i += 1
                chars.add(pattern[i])
            elif i + 2 < len(pattern) and pattern[i + 1] == '-' and pattern[i + 2] != ']':
                start = pattern[i]
                i += 2
                end = pattern[i]
                for c in range(ord(start), ord(end) + 1):
                    chars.add(chr(c))
            else:
                chars.add(pattern[i])
            i += 1
        return negated, chars, i + 1

    def recurse(pat_pos, path_pos):
        while pat_pos < len(pattern) and path_pos < len(path):
            c = pattern[pat_pos]
            if c == '*':
                if pat_pos + 1 < len(pattern) and pattern[pat_pos + 1] == '*':
                    # **
                    if pat_pos + 2 < len(pattern) and pattern[pat_pos + 2] == '/':
                        # Match zero or more whole segments
                        rest = pat_pos + 3
                        # Try matching rest at current position (zero segments)
                        if recurse(rest, path_pos):
                            return True
                        # Try consuming segments one by one
                        for j in range(path_pos, len(path)):
                            if path[j] == '/':
                                if recurse(rest, j + 1):
                                    return True
                        return False
                    else:
                        # ** at end or not followed by /
                        pat_pos += 2
                        continue
                else:
                    # Single * matches zero or more non-/ chars
                    rest = pat_pos + 1
                    for j in range(path_pos, len(path) + 1):
                        if j > path_pos and path[j - 1] == '/':
                            break
                        if recurse(rest, j):
                            return True
                    return False
            elif c == '?':
                if path[path_pos] == '/':
                    return False
                pat_pos += 1
                path_pos += 1
            elif c == '[':
                negated, chars, end = parse_class(pattern, pat_pos)
                ch = path[path_pos]
                if ch == '/':
                    return False
                in_class = ch in chars
                if negated:
                    if in_class:
                        return False
                else:
                    if not in_class:
                        return False
                pat_pos = end
                path_pos += 1
            elif c == '\\' and pat_pos + 1 < len(pattern):
                pat_pos += 1
                if pattern[pat_pos] != path[path_pos]:
                    return False
                pat_pos += 1
                path_pos += 1
            else:
                if c != path[path_pos]:
                    return False
                pat_pos += 1
                path_pos += 1
        # Consume trailing **/ patterns
        while pat_pos < len(pattern):
            if pattern[pat_pos:pat_pos + 3] == '**/':
                pat_pos += 3
            else:
                break
        return pat_pos == len(pattern) and path_pos == len(path)

    return recurse(0, 0)