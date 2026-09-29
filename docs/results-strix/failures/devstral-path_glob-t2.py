import re

def match(pattern, path):
    def escape_backslashes(s):
        return s.replace('\\', '\\\\')

    def parse_segment(segment):
        parts = []
        i = 0
        while i < len(segment):
            if i + 1 < len(segment) and segment[i] == '\\':
                parts.append(segment[i+1])
                i += 2
            elif segment[i] == '?':
                parts.append('.')
                i += 1
            elif segment[i] == '*':
                parts.append('.*')
                i += 1
            elif segment[i] == '[':
                j = segment.find(']', i)
                if j == -1:
                    return None
                class_ = segment[i+1:j]
                parts.append('[' + escape_backslashes(class_) + ']')
                i = j + 1
            else:
                parts.append(escape_backslashes(segment[i]))
                i += 1
        return ''.join(parts) if parts else None

    pattern_segments = pattern.split('/')
    path_segments = path.split('/')

    if len(pattern_segments) == 0:
        return len(path_segments) == 0

    if pattern_segments[0] == '**':
        if len(path_segments) == 0:
            return False
        for i in range(len(path_segments)):
            if match('/'.join(pattern_segments), '/'.join(path_segments[i:])):
                return True
        return False

    if len(pattern_segments) != len(path_segments):
        return False

    for p_seg, path_seg in zip(pattern_segments, path_segments):
        if p_seg == '**':
            return True
        if p_seg == '*':
            if not path_seg:
                return False
            continue
        if '*' in p_seg or '?' in p_seg or '[' in p_seg or '\\' in p_seg:
            regex = parse_segment(p_seg)
            if regex is None:
                return False
            if not re.fullmatch(regex, path_seg):
                return False
        else:
            if p_seg != path_seg:
                return False

    return True