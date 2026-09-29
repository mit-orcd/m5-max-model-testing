import re

def match(pattern, path):
    def escape_regex(s):
        return re.escape(s)

    def pattern_to_regex(pattern):
        i = 0
        n = len(pattern)
        result = []
        while i < n:
            c = pattern[i]
            if c == '\\':
                i += 1
                if i < n:
                    result.append(re.escape(pattern[i]))
                i += 1
            elif c == '*':
                if i + 1 < n and pattern[i+1] == '*':
                    result.append('(?:[^/]*/)*')
                    i += 2
                else:
                    result.append('[^/]*')
                    i += 1
            elif c == '?':
                result.append('[^/]')
                i += 1
            elif c == '[':
                j = i + 1
                if j < n and pattern[j] == '!':
                    j += 1
                    negate = True
                else:
                    negate = False
                while j < n and pattern[j] != ']':
                    j += 1
                if j >= n:
                    result.append(re.escape('['))
                    i += 1
                    continue
                content = pattern[i+1:j]
                if negate:
                    content = '^' + content
                result.append('[' + content + ']')
                i = j + 1
            else:
                result.append(re.escape(c))
                i += 1
        return '^' + ''.join(result) + '$'

    regex = pattern_to_regex(pattern)
    return bool(re.fullmatch(regex, path))