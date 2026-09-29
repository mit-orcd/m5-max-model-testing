def match(pattern, path):
    def match_segment(seg, pattern_segment):
        # Handle backslash escapes in pattern_segment
        i = 0
        n = len(pattern_segment)
        j = 0
        
        while i < len(seg) and j < n:
            if j < n and pattern_segment[j] == '\\':
                j += 1
                if j >= n:
                    return False
                if seg[i] != pattern_segment[j]:
                    return False
                i += 1
                j += 1
            elif pattern_segment[j] == '?':
                if seg[i] == '/':
                    return False
                i += 1
                j += 1
            elif pattern_segment[j] == '[':
                # Start of character class
                j += 1
                if j >= n:
                    return False
                negated = False
                if j < n and pattern_segment[j] == '!':
                    negated = True
                    j += 1
                # Find closing bracket
                k = j
                if k < n and pattern_segment[k] == ']':
                    k += 1
                while k < n and pattern_segment[k] != ']':
                    if pattern_segment[k] == '\\' and k + 1 < n:
                        k += 2
                    else:
                        k += 1
                if k >= n:
                    return False
                class_str = pattern_segment[j:k]
                j = k + 1
                # Check character class
                found = False
                c = seg[i]
                if c == '/':
                    return False
                idx = 0
                while idx < len(class_str):
                    if class_str[idx] == '\\' and idx + 1 < len(class_str):
                        idx += 1
                        if c == class_str[idx]:
                            found = True
                            break
                    elif idx + 2 < len(class_str) and class_str[idx + 1] == '-':
                        # Range
                        start = class_str[idx]
                        end = class_str[idx + 2]
                        if start <= c <= end:
                            found = True
                            break
                        idx += 3
                    else:
                        if c == class_str[idx]:
                            found = True
                            break
                        idx += 1
                if found == negated:
                    return False
                i += 1
            elif pattern_segment[j] == '*':
                return True  # '*' in segment matches rest of segment
            else:
                if seg[i] != pattern_segment[j]:
                    return False
                i += 1
                j += 1
        
        # If pattern segment is exhausted, check if segment is exhausted
        if j == n:
            return i == len(seg)
        
        # If segment is exhausted, check if pattern segment is all '*' wildcards
        while j < n:
            if pattern_segment[j] != '*':
                return False
            j += 1
        return True
    
    # Split pattern and path into segments
    pattern_segments = pattern.split('/')
    path_segments = path.split('/')
    
    # Handle leading/trailing slashes
    if pattern.startswith('/') != path.startswith('/'):
        return False
    if pattern.endswith('/') != path.endswith('/'):
        return False
    
    # Remove empty first segment if path starts with '/' to align with pattern
    if path.startswith('/'):
        path_segments = path_segments[1:] if path_segments[0] == '' else path_segments
    
    # Remove empty last segment if path ends with '/' to align with pattern
    if path.endswith('/') and path_segments and path_segments[-1] == '':
        path_segments = path_segments[:-1]
    
    # Handle leading empty segment in pattern (from leading '/')
    if pattern.startswith('/'):
        pattern_segments = pattern_segments[1:] if pattern_segments[0] == '' else pattern_segments
    
    # Handle trailing empty segment in pattern (from trailing '/')
    if pattern.endswith('/') and pattern_segments and pattern_segments[-1] == '':
        pattern_segments = pattern_segments[:-1]
    
    return match_segments(pattern_segments, path_segments)

def match_segments(pattern_segments, path_segments):
    # Use dynamic programming with memoization
    memo = {}
    
    def dp(pi, si):
        if (pi, si) in memo:
            return memo[(pi, si)]
        
        # Base cases
        if pi == len(pattern_segments) and si == len(path_segments):
            return True
        if pi == len(pattern_segments):
            return False
        if si == len(path_segments):
            # Check if remaining pattern segments are all '**' or empty strings
            for p in pattern_segments[pi:]:
                if p != '**' and p != '':
                    return False
            return True
        
        pc = pattern_segments[pi]
        
        if pc == '**':
            # Try matching zero segments
            if dp(pi + 1, si):
                memo[(pi, si)] = True
                return True
            # Try matching one or more segments
            for k in range(si, len(path_segments)):
                if dp(pi, k + 1):
                    memo[(pi, si)] = True
                    return True
            memo[(pi, si)] = False
            return False
        elif pc == '':
            # Empty segment only matches empty segment
            if si < len(path_segments) and path_segments[si] == '':
                result = dp(pi + 1, si + 1)
                memo[(pi, si)] = result
                return result
            else:
                memo[(pi, si)] = False
                return False
        else:
            # Normal segment
            if si >= len(path_segments):
                memo[(pi, si)] = False
                return False
            if match_segment(path_segments[si], pc):
                result = dp(pi + 1, si + 1)
                memo[(pi, si)] = result
                return result
            else:
                memo[(pi, si)] = False
                return False
    
    return dp(0, 0)