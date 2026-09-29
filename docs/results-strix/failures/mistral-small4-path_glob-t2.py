def match(pattern, path):
    def escape(s):
        return s.replace('\\', '\\\\')

    def split_path(p):
        parts = []
        current = []
        escaped = False
        for c in escape(p):
            if c == '\\':
                escaped = True
                continue
            if c == '/' and not escaped:
                parts.append(''.join(current))
                current = []
                escaped = False
            else:
                current.append(c)
                escaped = False
        if current:
            parts.append(''.join(current))
        return parts

    pattern_parts = split_path(pattern)
    path_parts = split_path(path)

    def match_segment(pat, seg):
        i = 0
        j = 0
        while i < len(pat) and j < len(seg):
            if pat[i] == '\\':
                if i + 1 >= len(pat):
                    return False
                if pat[i + 1] != seg[j]:
                    return False
                i += 2
                j += 1
            elif pat[i] == '?':
                if seg[j] == '/':
                    return False
                i += 1
                j += 1
            elif pat[i] == '*':
                if i + 1 < len(pat) and pat[i + 1] == '*':
                    if i + 2 < len(pat):
                        return False
                    i += 2
                    while j < len(seg):
                        if match_segment(pat[i:], seg[j:]):
                            return True
                        j += 1
                else:
                    while j < len(seg) and seg[j] != '/':
                        j += 1
                    i += 1
            else:
                if pat[i] != seg[j]:
                    return False
                i += 1
                j += 1
        return i == len(pat) and j == len(seg)

    def process_pattern(pat_parts, path_parts):
        i = 0
        j = 0
        while i < len(pat_parts) and j <= len(path_parts):
            if pat_parts[i] == '**':
                if i == len(pat_parts) - 1:
                    return True
                i += 1
                if i < len(pat_parts):
                    while j < len(path_parts):
                        if match_segment(pat_parts[i], path_parts[j]):
                            return process_pattern(pat_parts[i + 1:], path_parts[j + 1:])
                        j += 1
                    return False
            else:
                if j >= len(path_parts):
                    return False
                if not match_segment(pat_parts[i], path_parts[j]):
                    return False
                i += 1
                j += 1
        return i == len(pat_parts) and j == len(path_parts)

    return process_pattern(pattern_parts, path_parts)