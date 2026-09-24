def match(pattern, path):
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
            elif pat[i] == '[':
                if i + 1 >= len(pat):
                    return False
                i += 1
                negate = False
                if pat[i] == '!':
                    negate = True
                    i += 1
                matched = False
                while i < len(pat) and pat[i] != ']':
                    if i + 1 < len(pat) and pat[i + 1] == '-':
                        if i + 2 >= len(pat):
                            return False
                        start = pat[i]
                        end = pat[i + 2]
                        if start <= seg[j] <= end:
                            matched = True
                        i += 3
                    else:
                        if pat[i] == seg[j]:
                            matched = True
                        i += 1
                if i >= len(pat) or pat[i] != ']':
                    return False
                if negate:
                    if matched:
                        return False
                else:
                    if not matched:
                        return False
                i += 1
                j += 1
            elif pat[i] == '?':
                if seg[j] == '/':
                    return False
                i += 1
                j += 1
            elif pat[i] == '*':
                if i + 1 < len(pat) and pat[i + 1] == '*':
                    if i + 2 >= len(pat):
                        return True
                    if pat[i + 2] == '/':
                        i += 3
                        while j < len(seg):
                            j += 1
                    else:
                        return False
                else:
                    i += 1
                    while j < len(seg) and seg[j] != '/':
                        j += 1
            else:
                if pat[i] != seg[j]:
                    return False
                i += 1
                j += 1
        return i == len(pat) and j == len(seg)

    segments = path.split('/')
    pattern_segments = pattern.split('/')
    i = 0
    j = 0
    while i < len(pattern_segments) and j < len(segments):
        if pattern_segments[i] == '**':
            if i + 1 == len(pattern_segments):
                return True
            i += 1
            while i < len(pattern_segments) and j < len(segments):
                if match_segment(pattern_segments[i], segments[j]):
                    i += 1
                    j += 1
                else:
                    j += 1
        else:
            if not match_segment(pattern_segments[i], segments[j]):
                return False
            i += 1
            j += 1
    return i == len(pattern_segments) and j == len(segments)