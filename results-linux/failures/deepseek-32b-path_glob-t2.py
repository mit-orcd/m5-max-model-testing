import re

def match(pattern, path):
    def split_pattern(p):
        segments = []
        current = []
        i = 0
        while i < len(p):
            if p[i] == '/':
                if current:
                    segments.append(''.join(current))
                    current = []
                i += 1
                continue
            if p[i] == '\\':
                if i + 1 < len(p):
                    current.append(p[i + 1])
                    i += 2
                else:
                    current.append('\\')
                    i += 1
            else:
                current.append(p[i])
                i += 1
        if current:
            segments.append(''.join(current))
        return segments

    pattern_segments = split_pattern(pattern)
    path_segments = path.split('/')

    def match_segment(seg_pattern, seg_path):
        regex = ''
        i = 0
        while i < len(seg_pattern):
            if seg_pattern[i] == '*':
                regex += '[^/]*'
                i += 1
            elif seg_pattern[i] == '?':
                regex += '[^/]'
                i += 1
            elif seg_pattern[i] == '[':
                j = i + 1
                while j < len(seg_pattern) and seg_pattern[j] != ']':
                    j += 1
                if j >= len(seg_pattern):
                    regex += re.escape(seg_pattern[i])
                    i += 1
                else:
                    cc = seg_pattern[i + 1:j]
                    if cc.startswith('!'):
                        cc = '[^' + cc[1:] + ']'
                    else:
                        cc = '[' + re.escape(cc) + ']'
                    regex += cc
                    i = j + 1
            else:
                regex += re.escape(seg_pattern[i])
                i += 1
        return re.fullmatch(regex, seg_path) is not None

    def backtrack(i, j):
        if i == len(pattern_segments) and j == len(path_segments):
            return True
        if i == len(pattern_segments):
            return False
        if j == len(path_segments):
            for k in range(i, len(pattern_segments)):
                if pattern_segments[k] != '**':
                    return False
            return True
        current_p = pattern_segments[i]
        if current_p == '**':
            if backtrack(i + 1, j):
                return True
            for k in range(j, len(path_segments)):
                if backtrack(i + 1, k + 1):
                    return True
            return False
        else:
            if j >= len(path_segments):
                return False
            if match_segment(current_p, path_segments[j]):
                if backtrack(i + 1, j + 1):
                    return True
            return False

    return backtrack(0, 0)