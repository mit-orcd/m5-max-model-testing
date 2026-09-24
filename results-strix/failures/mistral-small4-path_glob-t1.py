import re

def match(pattern, path):
    def escape_segment(segment):
        i, n = 0, len(segment)
        escaped = []
        while i < n:
            c = segment[i]
            if c == '\\' and i + 1 < n:
                escaped.append(re.escape(segment[i + 1]))
                i += 2
            elif c == '?':
                escaped.append('[^/]')
                i += 1
            elif c == '*':
                if i + 1 < n and segment[i + 1] == '*':
                    escaped.append('.*')
                    i += 2
                else:
                    escaped.append('[^/]*')
                    i += 1
            else:
                escaped.append(re.escape(c))
                i += 1
        return ''.join(escaped)

    pattern_segments = pattern.split('/')
    path_segments = path.split('/')

    if len(pattern_segments) != len(path_segments):
        return False

    for pat, p in zip(pattern_segments, path_segments):
        if pat == '**':
            continue
        if not pat:
            if not p:
                continue
            return False
        regex = ''
        i = 0
        while i < len(pat):
            if pat[i] == '\\' and i + 1 < len(pat):
                regex += re.escape(pat[i + 1])
                i += 2
            elif pat[i] == '?':
                regex += '[^/]'
                i += 1
            elif pat[i] == '*':
                if i + 1 < len(pat) and pat[i + 1] == '*':
                    regex += '.*'
                    i += 2
                else:
                    regex += '[^/]*'
                    i += 1
            else:
                regex += re.escape(pat[i])
                i += 1
        if not re.fullmatch(regex, p):
            return False
    return True