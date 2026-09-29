def match(pattern, path):
    def escape(s):
        return ''.join(c if c not in '\\*?[]' else '\\' + c for c in s)

    def compile_pattern(p):
        i = 0
        n = len(p)
        res = []
        while i < n:
            c = p[i]
            if c == '?':
                res.append('[^/]')
                i += 1
            elif c == '*':
                if i + 1 < n and p[i + 1] == '*':
                    if i + 2 < n and p[i + 2] == '/':
                        res.append('(.*)')
                        i += 3
                        continue
                res.append('[^/]*')
                i += 1
            elif c == '[':
                j = i + 1
                if j < n and p[j] in '!^':
                    j += 1
                if j < n and p[j] == ']':
                    j += 1
                while j < n and p[j] != ']':
                    j += 1
                if j >= n:
                    res.append('\\[')
                else:
                    stuff = p[i+1:j]
                    if stuff[0] == '!':
                        stuff = '^' + stuff[1:]
                    elif stuff[0] == '^':
                        stuff = '\\' + stuff
                    res.append('[' + stuff + ']')
                    i = j + 1
                    continue
                i += 1
            elif c == '\\':
                if i + 1 < n:
                    res.append('\\' + p[i + 1])
                    i += 2
                else:
                    res.append('\\')
                    i += 1
            else:
                res.append(re.escape(c))
                i += 1
        return ''.join(res)

    import re
    regex_pattern = '^' + compile_pattern(pattern) + '$'
    return bool(re.match(regex_pattern, path))