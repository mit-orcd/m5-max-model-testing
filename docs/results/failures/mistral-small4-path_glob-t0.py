import re

def match(pattern, path):
    def escape_glob_to_regex(glob):
        regex = []
        i = 0
        n = len(glob)
        while i < n:
            c = glob[i]
            if c == '\\':
                if i + 1 < n:
                    regex.append(re.escape(glob[i+1]))
                    i += 2
                else:
                    regex.append(re.escape(c))
                    i += 1
            elif c == '*':
                if i + 1 < n and glob[i+1] == '*':
                    regex.append('.*')
                    i += 2
                else:
                    regex.append('[^/]*')
                    i += 1
            elif c == '?':
                regex.append('[^/]')
                i += 1
            elif c == '[':
                j = i + 1
                if j < n and glob[j] == '!':
                    regex.append('(?![/])[^/]')
                    i += 2
                else:
                    regex.append('[')
                    while j < n and glob[j] != ']':
                        if j + 1 < n and glob[j+1] == '-':
                            if j + 2 < n:
                                regex.append(glob[j:j+3])
                                j += 3
                            else:
                                regex.append(glob[j])
                                j += 1
                        else:
                            regex.append(glob[j])
                            j += 1
                    if j < n:
                        regex.append(glob[j])
                        j += 1
                    regex.append(']')
                    i = j
            else:
                regex.append(re.escape(c))
                i += 1
        return ''.join(regex)

    pattern_parts = pattern.split('/')
    path_parts = path.split('/')

    if len(pattern_parts) != len(path_parts):
        return False

    for p_part, path_part in zip(pattern_parts, path_parts):
        if p_part == '**':
            continue
        if not re.fullmatch(p_part, path_part):
            return False
    return True

def re(fullmatch, pattern):
    regex_pattern = '^' + pattern + '$'
    return re.compile(regex_pattern).fullmatch

re = lambda pattern: re.compile('^' + pattern + '$').fullmatch