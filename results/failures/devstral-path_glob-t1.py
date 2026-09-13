def match(pattern, path):
    def parse_pattern(pat):
        segments = []
        i = 0
        while i < len(pat):
            if pat[i] == '\\':
                segments.append(pat[i+1])
                i += 2
            elif pat[i] == '*':
                if i + 1 < len(pat) and pat[i+1] == '*':
                    segments.append(('**',))
                    i += 2
                else:
                    segments.append(('*',))
                    i += 1
            elif pat[i] == '?':
                segments.append('?')
                i += 1
            elif pat[i] == '[':
                j = pat.find(']', i)
                if j == -1:
                    raise ValueError("Unmatched '[' in pattern")
                bracket = pat[i+1:j]
                negation = False
                if bracket.startswith('!'):
                    negation = True
                    bracket = bracket[1:]
                if not bracket:
                    raise ValueError("Empty character class")
                parts = []
                k = 0
                while k < len(bracket):
                    if k + 2 < len(bracket) and bracket[k+1] == '-':
                        parts.append((bracket[k], bracket[k+2]))
                        k += 3
                    else:
                        parts.append(bracket[k])
                        k += 1
                segments.append(('[]', parts, negation))
                i = j + 1
            else:
                segments.append(pat[i])
                i += 1
        return segments

    def match_segment(seg, p_seg):
        if isinstance(seg, str):
            return seg == p_seg
        elif seg == '?':
            return len(p_seg) == 1 and p_seg != '/'
        elif seg == ('*',):
            return True
        elif seg == ('**',):
            return True
        elif seg[0] == '[]':
            parts, negation = seg[1], seg[2]
            if len(p_seg) != 1:
                return False
            for part in parts:
                if isinstance(part, tuple):
                    if ord(part[0]) <= ord(p_seg) <= ord(part[1]):
                        return not negation
                else:
                    if part == p_seg:
                        return not negation
            return negation
        return False

    pat_segments = parse_pattern(pattern)
    path_segments = path.split('/')

    def backtrack(pat_idx, path_idx):
        if pat_idx == len(pat_segments) and path_idx == len(path_segments):
            return True
        if pat_idx == len(pat_segments):
            return False
        if pat_segments[pat_idx] == ('**',):
            for k in range(path_idx, len(path_segments) + 1):
                if backtrack(pat_idx + 1, k):
                    return True
            return False
        if path_idx >= len(path_segments):
            return False
        if match_segment(pat_segments[pat_idx], path_segments[path_idx]):
            return backtrack(pat_idx + 1, path_idx + 1)
        return False

    return backtrack(0, 0)