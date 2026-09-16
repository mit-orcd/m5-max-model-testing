def match(pattern, path):
    if not path:
        return pattern == '' or pattern == '*' or pattern == '**' or (pattern.startswith('**/') and pattern[3:] == '')
    
    # Tokenize pattern into segments
    pattern_segments = []
    current = []
    i = 0
    while i < len(pattern):
        c = pattern[i]
        if c == '/':
            if current:
                pattern_segments.append(''.join(current))
                current = []
            pattern_segments.append('/')
        elif c == '*':
            if i + 1 < len(pattern) and pattern[i+1] == '*':
                if current:
                    pattern_segments.append(''.join(current))
                    current = []
                pattern_segments.append('**')
                i += 1
            else:
                current.append(c)
        elif c == '?':
            current.append(c)
        elif c == '[':
            j = i + 1
            neg = False
            if j < len(pattern) and pattern[j] == '!':
                neg = True
                j += 1
            # Find closing bracket
            end = j
            while end < len(pattern) and pattern[end] != ']':
                if pattern[end] == '\\' and end + 1 < len(pattern):
                    end += 2
                else:
                    end += 1
            if end < len(pattern):
                char_class = pattern[i:end+1]
                if current:
                    pattern_segments.append(''.join(current))
                    current = []
                pattern_segments.append(char_class)
                i = end
            else:
                current.append(c)
        elif c == '\\':
            if i + 1 < len(pattern):
                current.append(pattern[i+1])
                i += 1
            else:
                current.append(c)
        else:
            current.append(c)
        i += 1
    
    if current:
        pattern_segments.append(''.join(current))
    
    # Tokenize path into segments
    path_segments = path.split('/')
    
    # Compile pattern segments into a list of matchers
    def compile_segment(seg):
        if seg == '**':
            return ('**',)
        if seg == '/':
            return ('/',)
        if seg.startswith('[') and seg.endswith(']'):
            neg = False
            chars = set()
            idx = 1
            if idx < len(seg) and seg[idx] == '!':
                neg = True
                idx += 1
            while idx < len(seg):
                c = seg[idx]
                if c == '\\' and idx + 1 < len(seg):
                    idx += 1
                    chars.add(seg[idx])
                elif idx + 2 < len(seg) and seg[idx+1] == '-' and seg[idx+2] != ']':
                    start, end = seg[idx], seg[idx+2]
                    for code in range(ord(start), ord(end) + 1):
                        chars.add(chr(code))
                    idx += 3
                else:
                    chars.add(c)
                idx += 1
            return ('char_class', chars, neg)
        if '?' in seg or '*' in seg:
            return ('wildcard', seg)
        return ('literal', seg)
    
    compiled = [compile_segment(s) for s in pattern_segments]
    
    def match_segments(p_idx, s_idx):
        if p_idx == len(compiled) and s_idx == len(path_segments):
            return True
        if p_idx == len(compiled):
            return False
        if s_idx == len(path_segments):
            # Check if remaining pattern segments can match empty
            while p_idx < len(compiled):
                kind = compiled[p_idx][0]
                if kind == '**':
                    p_idx += 1
                elif kind == '/':
                    return False
                else:
                    return False
            return True
        
        p_kind = compiled[p_idx][0]
        
        if p_kind == '/':
            if s_idx < len(path_segments) and path_segments[s_idx] == '':
                return match_segments(p_idx + 1, s_idx + 1)
            return False
        
        if p_kind == '**':
            # Try matching zero segments
            if match_segments(p_idx + 1, s_idx):
                return True
            # Try matching one or more segments
            for i in range(s_idx, len(path_segments)):
                if match_segments(p_idx + 1, i + 1):
                    return True
            return False
        
        if p_kind == 'literal':
            if s_idx < len(path_segments) and path_segments[s_idx] == p_kind[1]:
                return match_segments(p_idx + 1, s_idx + 1)
            return False
        
        if p_kind == 'char_class':
            chars, neg = p_kind[1], p_kind[2]
            if s_idx < len(path_segments):
                seg = path_segments[s_idx]
                if len(seg) == 1:
                    c = seg
                    if neg:
                        if c not in chars:
                            return match_segments(p_idx + 1, s_idx + 1)
                    else:
                        if c in chars:
                            return match_segments(p_idx + 1, s_idx + 1)
            return False
        
        if p_kind == 'wildcard':
            seg = p_kind[1]
            if s_idx >= len(path_segments):
                return False
            path_seg = path_segments[s_idx]
            if '?' in seg or '*' in seg:
                # Convert to regex-like matching for single segment
                import re
                regex = '^'
                j = 0
                while j < len(seg):
                    c = seg[j]
                    if c == '\\':
                        if j + 1 < len(seg):
                            regex += re.escape(seg[j+1])
                            j += 2
                        else:
                            regex += re.escape(c)
                            j += 1
                    elif c == '?':
                        regex += '.'
                        j += 1
                    elif c == '*':
                        regex += '[^/]*'
                        j += 1
                    elif c == '[':
                        k = j + 1
                        if k < len(seg) and seg[k] == '!':
                            regex += '[^'
                            k += 1
                        else:
                            regex += '['
                        while k < len(seg) and seg[k] != ']':
                            if seg[k] == '\\' and k + 1 < len(seg):
                                regex += re.escape(seg[k+1])
                                k += 2
                            elif k + 2 < len(seg) and seg[k+1] == '-' and seg[k+2] != ']':
                                regex += re.escape(seg[k]) + '-' + re.escape(seg[k+2])
                                k += 3
                            else:
                                regex += re.escape(seg[k])
                                k += 1
                        if k < len(seg) and seg[k] == ']':
                            regex += ']'
                            j = k + 1
                        else:
                            regex += ']'
                            j = k
                    else:
                        regex += re.escape(c)
                        j += 1
                regex += '$'
                try:
                    if re.match(regex, path_seg):
                        return match_segments(p_idx + 1, s_idx + 1)
                except:
                    pass
            return False
        
        return False
    
    return match_segments(0, 0)