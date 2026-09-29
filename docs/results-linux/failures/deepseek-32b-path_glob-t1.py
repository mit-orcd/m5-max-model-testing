import re

def match(pattern, path):
    def split_pattern(s):
        segments = []
        current = []
        i = 0
        n = len(s)
        while i < n:
            if s[i] == '\\':
                if i + 1 < n:
                    current.append(s[i+1])
                    i += 2
                else:
                    current.append(s[i])
                    i += 1
            elif s[i] == '/':
                segments.append(''.join(current))
                current = []
                i += 1
            elif s[i] == '[':
                j = i + 1
                if j < n and s[j] == '!':
                    j += 1
                start = j
                while j < n and s[j] != ']':
                    if s[j] == '\\':
                        j += 1
                        if j < n:
                            j += 1
                    else:
                        j += 1
                if j < n:
                    content = s[start:j]
                    processed = []
                    k = 0
                    while k < len(content):
                        if content[k] == '\\':
                            if k + 1 < len(content):
                                processed.append(content[k])
                                processed.append(content[k+1])
                                k += 2
                            else:
                                processed.append(content[k])
                                k += 1
                        else:
                            processed.append(content[k])
                            k += 1
                    class_str = ''.join(processed)
                    if s[i+1] == '!':
                        class_str = '^' + class_str
                    current.append('[' + class_str + ']')
                    i = j + 1
                else:
                    current.append('[')
                    i += 1
            else:
                current.append(s[i])
                i += 1
        if current:
            segments.append(''.join(current))
        return segments

    def pattern_to_regex(segment):
        regex = []
        i = 0
        n = len(segment)
        while i < n:
            c = segment[i]
            if c == '*':
                regex.append('.*')
                i += 1
            elif c == '?':
                regex.append('.')
                i += 1
            elif c == '[':
                j = i + 1
                if j < n and segment[j] == '!':
                    j += 1
                start = j
                while j < n and segment[j] != ']':
                    if segment[j] == '\\':
                        j += 1
                        if j < n:
                            j += 1
                    else:
                        j += 1
                if j < n:
                    content = segment[start:j]
                    processed = []
                    k = 0
                    while k < len(content):
                        if content[k] == '\\':
                            if k + 1 < len(content):
                                processed.append(re.escape(content[k]))
                                processed.append(re.escape(content[k+1]))
                                k += 2
                            else:
                                processed.append(re.escape(content[k]))
                                k += 1
                        else:
                            processed.append(re.escape(content[k]))
                            k += 1
                    class_str = ''.join(processed)
                    if segment[i+1] == '!':
                        class_str = '^' + class_str
                    regex.append('[' + class_str + ']')
                    i = j + 1
                else:
                    regex.append(re.escape(c))
                    i += 1
            else:
                regex.append(re.escape(c))
                i += 1
        return '^' + ''.join(regex) + '$'

    pattern_segments = split_pattern(pattern)
    path_segments = path.split('/')

    def helper(p, path, pi, psi):
        if pi == len(p) and psi == len(path):
            return True
        if pi >= len(p) or psi >= len(path):
            return False

        p_seg = p[pi]
        if p_seg == '**':
            if helper(p, path, pi + 1, psi):
                return True
            while psi < len(path):
                if helper(p, path, pi + 1, psi + 1):
                    return True
                psi += 1
            return False
        else:
            regex = pattern_to_regex(p_seg)
            try:
                if re.fullmatch(regex, path[psi]):
                    return helper(p, path, pi + 1, psi + 1)
            except:
                return False
            return False

    return helper(pattern_segments, path_segments, 0, 0)