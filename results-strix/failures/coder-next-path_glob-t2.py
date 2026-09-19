def match(pattern, path):
    # Convert pattern and path into segments
    pattern_segments = pattern.split('/')
    path_segments = path.split('/')
    
    def match_segment(pattern_seg, path_seg):
        """Match a single path segment against a pattern segment."""
        if not pattern_seg:
            return not path_seg
            
        p_len = len(pattern_seg)
        s_len = len(path_seg)
        
        # Dynamic programming for segment matching
        dp = [[False] * (s_len + 1) for _ in range(p_len + 1)]
        dp[0][0] = True
        
        # Handle leading asterisks
        for i in range(1, p_len + 1):
            if pattern_seg[i-1] == '*':
                dp[i][0] = dp[i-1][0]
            else:
                break
        
        for i in range(1, p_len + 1):
            for j in range(1, s_len + 1):
                if pattern_seg[i-1] == '*':
                    dp[i][j] = dp[i-1][j] or dp[i][j-1]
                elif pattern_seg[i-1] == '?':
                    dp[i][j] = dp[i-1][j-1]
                elif pattern_seg[i-1] == '[':
                    # Find closing bracket
                    k = i
                    negated = False
                    if k < p_len and pattern_seg[k] == '!':
                        negated = True
                        k += 1
                    # Find closing bracket
                    bracket_end = k
                    while bracket_end < p_len and pattern_seg[bracket_end] != ']':
                        bracket_end += 1
                    if bracket_end >= p_len:
                        # No closing bracket, treat as literal
                        dp[i][j] = (pattern_seg[i-1] == path_seg[j-1]) and dp[i-1][j-1]
                    else:
                        # Process character class
                        class_chars = set()
                        k += 1  # skip '!' if present
                        while k < bracket_end:
                            if k+2 < bracket_end and pattern_seg[k+1] == '-':
                                # Range
                                start_char = pattern_seg[k]
                                end_char = pattern_seg[k+2]
                                # Handle escape sequences in ranges
                                if start_char == '\\':
                                    start_char = pattern_seg[k+1]
                                if end_char == '\\':
                                    end_char = pattern_seg[k+3]
                                for c in range(ord(start_char), ord(end_char)+1):
                                    class_chars.add(chr(c))
                                k += 3
                            else:
                                # Single character
                                c = pattern_seg[k]
                                if c == '\\':
                                    c = pattern_seg[k+1]
                                    k += 1
                                class_chars.add(c)
                                k += 1
                        # Check if current character matches
                        matched = path_seg[j-1] in class_chars
                        if negated:
                            matched = not matched
                        dp[i][j] = matched and dp[i-1][j-1]
                        # Skip to after closing bracket
                        i += bracket_end - i + 1
                        p_len = bracket_end + 1
                        break
                else:
                    # Handle escaped characters
                    c1 = pattern_seg[i-1]
                    if c1 == '\\':
                        c1 = pattern_seg[i] if i < p_len else ''
                        i += 1
                        p_len = len(pattern_seg)
                        dp[i][j] = (c1 == path_seg[j-1]) and dp[i-1][j-1]
                    else:
                        dp[i][j] = (c1 == path_seg[j-1]) and dp[i-1][j-1]
            else:
                continue
            break
        
        return dp[p_len][s_len]
    
    def match_pattern(pattern_segments, path_segments):
        """Match pattern segments against path segments."""
        p_idx = 0
        s_idx = 0
        
        while p_idx < len(pattern_segments) and s_idx < len(path_segments):
            p_seg = pattern_segments[p_idx]
            
            if p_seg == '**':
                # Try matching 0 or more segments
                if p_idx == len(pattern_segments) - 1:
                    # ** at the end matches everything remaining
                    return True
                # Try matching 0 segments first
                if match_pattern(pattern_segments[p_idx+1:], path_segments[s_idx:]):
                    return True
                # Try matching 1 or more segments
                for k in range(1, len(path_segments) - s_idx + 1):
                    # Check if remaining pattern matches the rest
                    if match_pattern(pattern_segments[p_idx+1:], path_segments[s_idx+k:]):
                        return True
                return False
            
            # Normal segment matching
            if not match_segment(p_seg, path_segments[s_idx]):
                return False
            p_idx += 1
            s_idx += 1
        
        # Handle trailing ** segments
        while p_idx < len(pattern_segments) and pattern_segments[p_idx] == '**':
            p_idx += 1
        
        return p_idx == len(pattern_segments) and s_idx == len(path_segments)
    
    return match_pattern(pattern_segments, path_segments)