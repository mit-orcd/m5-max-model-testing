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
                    res.append(s[i])
                    i += 1
            else:
                res.append(s[i])
                i += 1
        return ''.join(res)

    pattern = escape(pattern)
    path = escape(path)

    def match_segment(pat, s):
        if not pat:
            return not s
        if pat == '*':
            return True
        if pat == '?':
            return len(s) == 1
        if pat[0] == '[':
            return match_class(pat, s)
        if pat[0] == '*':
            for i in range(len(s) + 1):
                if match_segment(pat[1:], s[i:]):
                    return True
            return False
        return s and s[0] == pat[0] and match_segment(pat[1:], s[1:])

    def match_class(pat, s):
        if not pat.startswith('['):
            return False
        pat = pat[1:]
        negate = False
        if pat.startswith('!'):
            negate = True
            pat = pat[1:]
        if pat.startswith(']'):
            return False
        if not s:
            return False
        i = 0
        in_range = False
        start = None
        while i < len(pat):
            if pat[i] == ']':
                if i + 1 < len(pat):
                    return False
                break
            if in_range:
                if pat[i] == '-':
                    if start is None:
                        return False
                    end = pat[i+1]
                    if end == ']':
                        return False
                    for c in range(ord(start), ord(end) + 1):
                        if chr(c) == s[0]:
                            return not negate
                    i += 2
                    in_range = False
                    start = None
                else:
                    if chr(ord(start)) == s[0]:
                        return not negate
                    i += 1
                    in_range = False
                    start = None
            else:
                if pat[i] == '-':
                    return False
                if pat[i+1:] and pat[i+1] == '-':
                    start = pat[i]
                    i += 2
                    in_range = True
                else:
                    if pat[i] == s[0]:
                        return not negate
                    i += 1
        return False

    pat_parts = pattern.split('/')
    path_parts = path.split('/')

    def match_parts(pat_idx, path_idx):
        if pat_idx == len(pat_parts):
            return path_idx == len(path_parts)
        if pat_parts[pat_idx] == '**':
            for skip in range(len(path_parts) - path_idx + 1):
                if match_parts(pat_idx + 1, path_idx + skip):
                    return True
            return False
        if path_idx >= len(path_parts):
            return False
        if match_segment(pat_parts[pat_idx], path_parts[path_idx]):
            return match_parts(pat_idx + 1, path_idx + 1)
        return False

    return match_parts(0, 0)