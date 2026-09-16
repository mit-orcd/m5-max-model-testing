def match(pattern, path):
    def parse_segment(seg):
        parts = []
        i = 0
        n = len(seg)
        while i < n:
            if seg[i] == '\\':
                parts.append(seg[i+1])
                i += 2
            elif seg[i] == '?':
                parts.append('?')
                i += 1
            elif seg[i] == '*':
                parts.append('*')
                i += 1
            elif seg[i] == '[':
                j = i + 1
                negate = False
                if j < n and seg[j] == '!':
                    negate = True
                    j += 1
                chars = []
                while j < n and seg[j] != ']':
                    if j + 2 < n and seg[j+1] == '-':
                        chars.append((seg[j], seg[j+2]))
                        j += 3
                    else:
                        chars.append(seg[j])
                        j += 1
                parts.append(('[', chars, negate))
                i = j + 1
            else:
                parts.append(seg[i])
                i += 1
        return parts

    def match_segment(seg_parts, path_seg):
        i = 0
        for part in seg_parts:
            if i >= len(path_seg):
                return False
            if part == '*':
                for j in range(i, len(path_seg) + 1):
                    if match_segment(seg_parts[1:], path_seg[j:]):
                        return True
                return False
            elif part == '?':
                i += 1
            elif isinstance(part, tuple) and part[0] == '[':
                _, chars, negate = part
                c = path_seg[i]
                matched = False
                for char in chars:
                    if isinstance(char, tuple):
                        if char[0] <= c <= char[1]:
                            matched = True
                            break
                    else:
                        if c == char:
                            matched = True
                            break
                if negate:
                    matched = not matched
                if not matched:
                    return False
                i += 1
            else:
                if path_seg[i] != part:
                    return False
                i += 1
        return i == len(path_seg)

    pattern_parts = pattern.split('/')
    path_parts = path.split('/')

    def dfs(p_idx, pt_idx):
        if p_idx == len(pattern_parts) and pt_idx == len(path_parts):
            return True
        if p_idx >= len(pattern_parts):
            return False
        if pattern_parts[p_idx] == '**':
            for i in range(pt_idx, len(path_parts) + 1):
                if dfs(p_idx + 1, i):
                    return True
            return False
        if pt_idx >= len(path_parts):
            return False
        seg_parts = parse_segment(pattern_parts[p_idx])
        if match_segment(seg_parts, path_parts[pt_idx]):
            return dfs(p_idx + 1, pt_idx + 1)
        return False

    return dfs(0, 0)