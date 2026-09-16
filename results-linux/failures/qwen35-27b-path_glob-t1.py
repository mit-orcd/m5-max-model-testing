def match(pattern, path):
    def escape_chars(s):
        res = []
        i = 0
        while i < len(s):
            if s[i] == '\\':
                if i + 1 < len(s):
                    res.append(s[i + 1])
                    i += 2
                else:
                    res.append('\\')
                    i += 1
            else:
                res.append(s[i])
                i += 1
        return ''.join(res)

    def char_match(c, ch):
        if c == ch:
            return True
        if c == '?' and ch != '/':
            return True
        return False

    def in_class(c, class_str):
        if not class_str:
            return False
        neg = False
        idx = 0
        if class_str[0] == '!':
            neg = True
            idx = 1
        elif class_str[0] == '^':
            neg = True
            idx = 1
        
        matched = False
        i = idx
        while i < len(class_str):
            if i + 2 < len(class_str) and class_str[i + 1] == '-':
                start = class_str[i]
                end = class_str[i + 2]
                if start <= c <= end:
                    matched = True
                i += 3
            else:
                if class_str[i] == c:
                    matched = True
                i += 1
        return not matched if neg else matched

    def match_segment(seg_pat, seg_path):
        if not seg_path:
            return not seg_pat
        if not seg_pat:
            return False
        
        i, j = 0, 0
        star_i = -1
        star_j = -1
        
        while j < len(seg_path):
            if i < len(seg_pat) and (seg_pat[i] == '*' or char_match(seg_pat[i], seg_path[j])):
                if seg_pat[i] == '*':
                    star_i = i + 1
                    star_j = j
                    i += 1
                else:
                    i += 1
                    j += 1
            elif star_i != -1:
                i = star_i
                j = star_j + 1
                star_j = j
            else:
                return False
        
        while i < len(seg_pat) and seg_pat[i] == '*':
            i += 1
        
        return i == len(seg_pat)

    def match_segment_with_brackets(seg_pat, seg_path):
        if not seg_path:
            return not seg_pat
        if not seg_pat:
            return False
        
        i, j = 0, 0
        star_i = -1
        star_j = -1
        
        while j < len(seg_path):
            if i < len(seg_pat):
                c = seg_pat[i]
                if c == '*':
                    star_i = i + 1
                    star_j = j
                    i += 1
                elif c == '[':
                    bracket_end = seg_pat.find(']', i + 1)
                    if bracket_end == -1:
                        return False
                    if in_class(seg_path[j], seg_pat[i+1:bracket_end]):
                        i = bracket_end + 1
                        j += 1
                    elif star_i != -1:
                        i = star_i
                        j = star_j + 1
                        star_j = j
                    else:
                        return False
                elif char_match(seg_pat[i], seg_path[j]):
                    i += 1
                    j += 1
                else:
                    if star_i != -1:
                        i = star_i
                        j = star_j + 1
                        star_j = j
                    else:
                        return False
            else:
                if star_i != -1:
                    i = star_i
                    j = star_j + 1
                    star_j = j
                else:
                    return False
        
        while i < len(seg_pat) and seg_pat[i] == '*':
            i += 1
        
        return i == len(seg_path)

    pattern = escape_chars(pattern)
    path = escape_chars(path)
    
    pat_parts = pattern.split('/')
    path_parts = path.split('/')
    
    def recurse(pi, path_idx):
        if pi == len(pat_parts) and path_idx == len(path_parts):
            return True
        if pi == len(pat_parts) or path_idx == len(path_parts):
            return False
        
        pat = pat_parts[pi]
        path_seg = path_parts[path_idx]
        
        if pat == '**':
            if recurse(pi + 1, path_idx):
                return True
            if recurse(pi, path_idx + 1):
                return True
            return False
        
        if match_segment_with_brackets(pat, path_seg):
            return recurse(pi + 1, path_idx + 1)
        
        return False

    return recurse(0, 0)