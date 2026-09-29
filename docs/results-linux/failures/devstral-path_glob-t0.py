def match(pattern, path):
    def escape(s):
        i = 0
        res = []
        while i < len(s):
            if s[i] == '\\':
                if i + 1 < len(s):
                    res.append(s[i+1])
                    i += 2
                else:
                    res.append('\\')
                    i += 1
            else:
                res.append(s[i])
                i += 1
        return ''.join(res)

    pattern = escape(pattern)
    path = escape(path)

    def match_segment(pat, seg):
        if not pat:
            return not seg
        if pat == '*':
            return True
        if pat == '?':
            return len(seg) == 1
        if pat.startswith('['):
            i = 1
            negate = False
            if i < len(pat) and pat[i] == '!':
                negate = True
                i += 1
            chars = []
            while i < len(pat) and pat[i] != ']':
                if i + 2 < len(pat) and pat[i+1] == '-':
                    chars.append((pat[i], pat[i+2]))
                    i += 3
                else:
                    chars.append((pat[i], pat[i]))
                    i += 1
            if i >= len(pat) or pat[i] != ']':
                return False
            if not seg:
                return False
            c = seg[0]
            matched = False
            for a, b in chars:
                if a <= c <= b:
                    matched = True
                    break
            if negate:
                matched = not matched
            return matched and len(seg) == 1
        if pat[0] == seg[0] and match_segment(pat[1:], seg[1:]):
            return True
        return False

    pat_parts = pattern.split('/')
    path_parts = path.split('/')

    i = 0
    j = 0
    star_star = False
    star_star_pos = -1

    while i < len(pat_parts) and j < len(path_parts):
        if pat_parts[i] == '**':
            star_star = True
            star_star_pos = i
            i += 1
            while i < len(pat_parts) and pat_parts[i] == '':
                i += 1
            continue
        if match_segment(pat_parts[i], path_parts[j]):
            i += 1
            j += 1
        elif star_star:
            i += 1
        else:
            return False

    while i < len(pat_parts) and pat_parts[i] == '':
        i += 1

    if i == len(pat_parts):
        while j < len(path_parts) and path_parts[j] == '':
            j += 1
        return j == len(path_parts)

    if star_star:
        while i < len(pat_parts):
            if pat_parts[i] == '**':
                i += 1
                while i < len(pat_parts) and pat_parts[i] == '':
                    i += 1
                continue
            if not match_segment(pat_parts[i], path_parts[j]):
                return False
            i += 1
            j += 1
        while j < len(path_parts) and path_parts[j] == '':
            j += 1
        return j == len(path_parts)

    return False