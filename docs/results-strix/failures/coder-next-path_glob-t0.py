def match(pattern, path):
    def match_segment(p_seg, seg):
        # Handle backslash escapes
        i = 0
        p_len = len(p_seg)
        s_len = len(seg)
        while i < p_len:
            if p_seg[i] == '\\':
                i += 1
                if i >= p_len:
                    return False
                if i >= s_len or p_seg[i] != seg[i-1]:
                    return False
                i += 1
                continue
            elif p_seg[i] == '?':
                if i >= s_len or seg[i] == '/':
                    return False
                i += 1
                continue
            elif p_seg[i] == '[':
                # Find closing bracket
                j = i + 1
                neg = False
                if j < p_len and p_seg[j] == '!':
                    neg = True
                    j += 1
                # Handle ] as first char in class
                if j < p_len and p_seg[j] == ']':
                    j += 1
                while j < p_len and p_seg[j] != ']':
                    j += 1
                if j >= p_len:
                    return False
                class_str = p_seg[i+1:j]
                if i+1 < j and p_seg[i+1] == '!':
                    class_str = class_str[1:]
                if i >= s_len or seg[i] == '/':
                    return False
                char = seg[i]
                matched = False
                k = 0
                while k < len(class_str):
                    if k + 2 < len(class_str) and class_str[k+1] == '-' and class_str[k+2] != ']':
                        # Range
                        start = class_str[k]
                        end = class_str[k+2]
                        if start <= char <= end:
                            matched = True
                            break
                        k += 3
                    else:
                        if class_str[k] == char:
                            matched = True
                            break
                        k += 1
                if neg:
                    matched = not matched
                if not matched:
                    return False
                i += 1
                continue
            elif p_seg[i] == '*':
                # '*' matches zero or more non-slash characters
                # We'll handle this by trying all possibilities
                for j in range(len(seg) - i + 1):
                    if match_segment(p_seg[i+1:], seg[i+j:]):
                        return True
                return False
            else:
                if i >= s_len or p_seg[i] != seg[i]:
                    return False
                i += 1
        return i == s_len
    
    # Split pattern and path by '/'
    p_parts = []
    i = 0
    while i < len(pattern):
        if pattern[i] == '\\':
            if i + 1 < len(pattern):
                p_parts.append(pattern[i+1])
                i += 2
                continue
            else:
                p_parts.append('\\')
                i += 1
                continue
        elif pattern[i] == '/':
            p_parts.append('/')
            i += 1
            continue
        else:
            # Collect segment until next '/' or end
            j = i
            while j < len(pattern) and pattern[j] != '/':
                j += 1
            p_parts.append(pattern[i:j])
            i = j
    
    # Split path by '/'
    path_parts = path.split('/') if path else []
    
    # Process pattern parts
    p_idx = 0
    path_idx = 0
    
    while p_idx < len(p_parts) and path_idx < len(path_parts):
        p_part = p_parts[p_idx]
        
        if p_part == '**':
            # '**' matches zero or more whole segments
            if p_idx == len(p_parts) - 1:
                # '**' at the end matches everything remaining
                return True
            # Try matching 0, 1, 2, ... segments
            for k in range(len(path_parts) - path_idx + 1):
                if match_segment(p_parts[p_idx+1], '/'.join(path_parts[path_idx+k:]) if k == 0 else path_parts[path_idx+k]):
                    # Continue matching the rest
                    if match_recursive(p_parts[p_idx+1:], path_parts[path_idx+k:]):
                        return True
            return False
        elif p_part == '/':
            # This shouldn't happen in a valid pattern, but handle it
            p_idx += 1
            continue
        else:
            # Regular segment pattern
            if path_idx >= len(path_parts):
                return False
            if not match_segment(p_part, path_parts[path_idx]):
                return False
            p_idx += 1
            path_idx += 1
    
    # Handle remaining '**' patterns
    while p_idx < len(p_parts) and p_parts[p_idx] == '**':
        p_idx += 1
    
    return p_idx == len(p_parts) and path_idx == len(path_parts)


def match_recursive(p_parts, path_parts):
    """Helper for recursive matching after '**'"""
    p_idx = 0
    path_idx = 0
    
    while p_idx < len(p_parts) and path_idx < len(path_parts):
        p_part = p_parts[p_idx]
        
        if p_part == '**':
            # '**' matches zero or more whole segments
            if p_idx == len(p_parts) - 1:
                return True
            for k in range(len(path_parts) - path_idx + 1):
                if match_segment(p_parts[p_idx+1], '/'.join(path_parts[path_idx+k:]) if k == 0 else path_parts[path_idx+k]):
                    if match_recursive(p_parts[p_idx+1:], path_parts[path_idx+k:]):
                        return True
            return False
        else:
            if path_idx >= len(path_parts):
                return False
            if not match_segment(p_part, path_parts[path_idx]):
                return False
            p_idx += 1
            path_idx += 1
    
    # Handle remaining '**' patterns
    while p_idx < len(p_parts) and p_parts[p_idx] == '**':
        p_idx += 1
    
    return p_idx == len(p_parts) and path_idx == len(path_parts)