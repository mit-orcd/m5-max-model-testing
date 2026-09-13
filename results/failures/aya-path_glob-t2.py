def match(pattern, path):
    def match_helper(p, pat):
        if p == pat:
            return True
        if pat[0] == '*':
            for i in range(len(p), len(pat), 1):
                if match_helper(p, pat[i:]):
                    return True
            return False
        if pat[0] == '[':
            for i in range(1, len(pat)):
                if pat[i] == ']':
                    break
            else:
                return False
            class_str = pat[1:i]
            if pat[0] == '!':
                return not class_str.find(p) != -1
            else:
                return class_str.find(p) != -1
        if pat[0] == '\\':
            return match_helper(p[1:], pat[1:])
        return False

    path_parts = path.split('/')
    pattern_parts = pattern.split('/')
    if len(path_parts) != len(pattern_parts):
        return False
    for i in range(len(path_parts)):
        if not match_helper(path_parts[i], pattern_parts[i]):
            return False
    return True