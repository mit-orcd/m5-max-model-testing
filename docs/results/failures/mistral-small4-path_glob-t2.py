def match(pattern, path):
    import re
    segments = path.split('/')
    pattern_segments = pattern.split('/')
    return _match_segments(pattern_segments, segments)

def _match_segments(pattern_segments, segments):
    if not pattern_segments and not segments:
        return True
    if not pattern_segments:
        return False
    if not segments:
        return pattern_segments[0] == '**' and _match_segments(pattern_segments[1:], segments)

    if pattern_segments[0] == '**':
        return _match_segments(pattern_segments[1:], segments) or _match_segments(pattern_segments, segments[1:]) or (
            len(segments) >= 1 and _match_segments(pattern_segments, segments[1:])
        )
    elif pattern_segments[0] == '*':
        return _match_segments(pattern_segments[1:], segments) or (
            len(segments) >= 1 and _match_segments(pattern_segments, segments[1:])
        )
    elif '?' in pattern_segments[0] or '[' in pattern_segments[0]:
        if not _match_segment(pattern_segments[0], segments[0]):
            return False
        return _match_segments(pattern_segments[1:], segments[1:])
    else:
        if pattern_segments[0] != segments[0]:
            return False
        return _match_segments(pattern_segments[1:], segments[1:])

def _match_segment(pattern, segment):
    pattern = pattern.replace('.', r'\.')
    pattern = pattern.replace('+', r'\+')
    pattern = pattern.replace('-', r'\-')

    def escape_regex(s):
        s = s.replace('\\', r'\\')
        s = s.replace('[', r'\[')
        s = s.replace(']', r'\]')
        s = s.replace('?', r'\?')
        s = s.replace('*', r'\*')
        s = s.replace('/', r'\/')
        return s

    escaped_segment = escape_regex(segment)
    i = 0
    n = len(pattern)
    while i < n:
        if pattern[i] == '\\':
            if i + 1 >= n:
                return False
            if pattern[i + 1] == '*':
                escaped_segment = escaped_segment.replace('*', '\*')
                i += 2
            elif pattern[i + 1] == '?':
                escaped_segment = escaped_segment.replace('?', '\?')
                i += 2
            else:
                escaped_segment = escaped_segment.replace(pattern[i + 1], re.escape(pattern[i + 1]))
                i += 2
        elif pattern[i] == '[':
            j = pattern.find(']', i)
            if j == -1:
                return False
            negate = False
            if pattern[i + 1] == '!':
                negate = True
                i += 1
            chars = pattern[i + 1:j]
            if negate:
                chars = '^' + chars
            pattern_class = chars
            if not re.match(f'[{pattern_class}]', segment):
                if negate:
                    if re.match(f'[{chars}]', segment):
                        return False
                else:
                    return False
            i = j + 1
        elif pattern[i] == '?':
            if len(segment) != 1:
                return False
            i += 1
        elif pattern[i] == '*':
            if i + 1 < n and pattern[i + 1] != '*':
                rest_pattern = pattern[i + 1:]
                if not _match_segment(rest_pattern, segment):
                    return False
                i += 1
            else:
                j = i
                while j < n and pattern[j] == '*':
                    j += 1
                if j >= n:
                    return True
                next_segment = segments[0]
                next_pattern = pattern[j]
                for k in range(len(segment) + 1):
                    if _match_segment(pattern[i:], segment[k:]):
                        return True
                return False
        else:
            if not re.match(re.escape(pattern[i]), segment):
                return False
            i += 1
    return True