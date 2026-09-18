def match(pattern, path):
    def compile_pattern(pat):
        i = 0
        segments = []
        while i < len(pat):
            if pat[i] == '\\':
                i += 1
                if i < len(pat):
                    segments.append(('lit', pat[i]))
                    i += 1
                else:
                    segments.append(('lit', '\\'))
            elif pat[i] == '?':
                segments.append(('single', None))
                i += 1
            elif pat[i] == '*':
                if i + 1 < len(pat) and pat[i + 1] == '*':
                    segments.append(('star_segment', None))
                    i += 2
                else:
                    segments.append(('star', None))
                    i += 1
            elif pat[i] == '[':
                i += 1
                negate = False
                if i < len(pat) and pat[i] == '!':
                    negate = True
                    i += 1
                chars = set()
                while i < len(pat) and pat[i] != ']':
                    if i + 2 < len(pat) and pat[i + 1] == '-':
                        start = pat[i]
                        end = pat[i + 2]
                        for c in range(ord(start), ord(end) + 1):
                            chars.add(chr(c))
                        i += 3
                    else:
                        chars.add(pat[i])
                        i += 1
                i += 1
                if negate:
                    segments.append(('class_not', chars))
                else:
                    segments.append(('class', chars))
            else:
                segments.append(('lit', pat[i]))
                i += 1
        return segments

    def match_single(char, cls):
        if char == '/':
            return False
        if cls is None:
            return True
        if cls[0] == 'class':
            return char in cls[1]
        elif cls[0] == 'class_not':
            return char not in cls[1]
        return True

    segments = compile_pattern(pattern)
    path_segs = path.split('/')

    memo = {}

    def dp(p_idx, s_idx):
        if (p_idx, s_idx) in memo:
            return memo[(p_idx, s_idx)]

        if p_idx == len(segments):
            result = s_idx == len(path_segs)
            memo[(p_idx, s_idx)] = result
            return result

        seg = segments[p_idx]

        if seg[0] == 'star_segment':
            if s_idx < len(path_segs):
                if dp(p_idx, s_idx + 1):
                    memo[(p_idx, s_idx)] = True
                    return True
            if dp(p_idx + 1, s_idx):
                memo[(p_idx, s_idx)] = True
                return True
            memo[(p_idx, s_idx)] = False
            return False

        if s_idx >= len(path_segs):
            memo[(p_idx, s_idx)] = False
            return False

        if seg[0] == 'star':
            if dp(p_idx + 1, s_idx):
                memo[(p_idx, s_idx)] = True
                return True
            if match_single(path_segs[s_idx], seg[1]):
                if dp(p_idx, s_idx + 1):
                    memo[(p_idx, s_idx)] = True
                    return True
            memo[(p_idx, s_idx)] = False
            return False

        if seg[0] == 'single':
            if match_single(path_segs[s_idx], seg[1]):
                if dp(p_idx + 1, s_idx + 1):
                    memo[(p_idx, s_idx)] = True
                    return True
            memo[(p_idx, s_idx)] = False
            return False

        if seg[0] in ('class', 'class_not'):
            if match_single(path_segs[s_idx], seg[1]):
                if dp(p_idx + 1, s_idx + 1):
                    memo[(p_idx, s_idx)] = True
                    return True
            memo[(p_idx, s_idx)] = False
            return False

        if seg[0] == 'lit':
            if path_segs[s_idx] == seg[1]:
                if dp(p_idx + 1, s_idx + 1):
                    memo[(p_idx, s_idx)] = True
                    return True
            memo[(p_idx, s_idx)] = False
            return False

        memo[(p_idx, s_idx)] = False
        return False

    return dp(0, 0)