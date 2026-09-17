def match(pattern, path):
    def _process_escapes(s):
        processed = []
        i = 0
        n = len(s)
        while i < n:
            if s[i] == '\\':
                i += 1
                if i < n:
                    processed.append(s[i])
                else:
                    processed.append('\\')
                i += 1
            else:
                processed.append(s[i])
                i += 1
        return ''.join(processed)

    def _matches_segment(pat, seg):
        i = j = 0
        len_pat = len(pat)
        len_seg = len(seg)
        while i < len_pat and j < len_seg:
            pc = pat[i]
            sc = seg[j]
            if pc == '?':
                i += 1
                j += 1
            elif pc == '*':
                for k in range(j, len_seg + 1):
                    if _matches_segment(pat[i+1:], seg[k:]):
                        return True
                return False
            elif pc == '[':
                i += 1
                negated = False
                if i < len_pat and pat[i] == '!':
                    negated = True
                    i += 1
                class_chars = []
                class_ranges = []
                while i < len_pat and pat[i] != ']':
                    if i + 1 < len_pat and pat[i+1] == '-':
                        start = pat[i]
                        end = pat[i+2]
                        class_ranges.append((start, end))
                        i += 3
                    else:
                        class_chars.append(pat[i])
                        i += 1
                if i >= len_pat:
                    return False
                i += 1
                match = sc in class_chars
                if not match:
                    for start, end in class_ranges:
                        if start <= sc <= end:
                            match = True
                            break
                if negated:
                    match = not match
                if not match:
                    return False
                j += 1
            else:
                if pc != sc:
                    return False
                i += 1
                j += 1
        return i == len_pat and j == len_seg

    def helper(p_idx, s_idx):
        if p_idx == len(pattern_segments):
            return s_idx == len(path_segments)
        if s_idx == len(path_segments):
            return all(seg == '**' for seg in pattern_segments[p_idx:])
        current_pat = pattern_segments[p_idx]
        if current_pat == '**':
            return helper(p_idx + 1, s_idx) or helper(p_idx, s_idx + 1)
        else:
            if s_idx >= len(path_segments):
                return False
            return _matches_segment(current_pat, path_segments[s_idx]) and helper(p_idx + 1, s_idx + 1)

    cleaned_pattern = _process_escapes(pattern)
    pattern_segments = cleaned_pattern.split('/')
    path_segments = path.split('/')
    return helper(0, 0)