import re

def match(pattern, path):
    def parse_pattern(p):
        i = 0
        n = len(p)
        parts = []
        while i < n:
            if p[i] == '\\':
                i += 1
                if i < n:
                    parts.append(re.escape(p[i]))
                    i += 1
                else:
                    parts.append(re.escape('\\'))
            elif p[i] == '[':
                j = i + 1
                if j < n and p[j] == '!':
                    j += 1
                    negate = True
                else:
                    negate = False
                if j < n and p[j] == ']':
                    j += 1
                while j < n and p[j] != ']':
                    j += 1
                if j >= n:
                    parts.append(re.escape('['))
                    i += 1
                    continue
                content = p[i+1:j]
                if negate:
                    content = '^' + content
                else:
                    content = content
                parts.append('[' + content + ']')
                i = j + 1
            elif p[i] == '*':
                if i + 1 < n and p[i+1] == '*':
                    parts.append('(?:.*/)?')
                    i += 2
                else:
                    parts.append('[^/]*')
                    i += 1
            elif p[i] == '?':
                parts.append('[^/]')
                i += 1
            else:
                parts.append(re.escape(p[i]))
                i += 1
        return ''.join(parts)

    regex_str = '^' + parse_pattern(pattern) + '$'
    try:
        return bool(re.fullmatch(regex_str, path))
    except re.error:
        return False