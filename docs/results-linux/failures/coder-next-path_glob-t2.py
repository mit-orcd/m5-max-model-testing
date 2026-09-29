def match(pattern, path):
    def match_segment(p, s):
        """Match a pattern segment (without '**') against a path segment."""
        pi = 0
        si = 0
        p_len = len(p)
        s_len = len(s)
        
        while si < s_len or pi < p_len:
            if pi < p_len and si < s_len and p[pi] == '\\':
                # Escaped character
                pi += 1
                if pi >= p_len or p[pi] != s[si]:
                    return False
                pi += 1
                si += 1
            elif pi < p_len and p[pi] == '?':
                if si >= s_len or s[si] == '/':
                    return False
                pi += 1
                si += 1
            elif pi < p_len and p[pi] == '[':
                # Character class
                pi += 1
                if pi >= p_len:
                    return False
                
                # Check for negation
                negated = False
                if pi < p_len and p[pi] == '!':
                    negated = True
                    pi += 1
                
                # Collect characters in class
                class_chars = []
                # Handle first char as literal if it's ']' or '-'
                if pi < p_len and p[pi] == ']' or (pi+1 < p_len and p[pi+1] == '-'):
                    class_chars.append(p[pi])
                    pi += 1
                
                # Parse the class
                while pi < p_len and p[pi] != ']':
                    if pi+2 < p_len and p[pi+1] == '-' and p[pi+2] != ']':
                        # Range
                        start = p[pi]
                        pi += 2
                        end = p[pi]
                        pi += 1
                        if start > end:
                            # Invalid range, treat as literal
                            class_chars.append(start)
                            class_chars.append('-')
                            class_chars.append(end)
                        else:
                            for c in range(ord(start), ord(end)+1):
                                class_chars.append(chr(c))
                    else:
                        class_chars.append(p[pi])
                        pi += 1
                
                if pi >= p_len:
                    return False
                
                pi += 1  # skip ']'
                
                # Check match
                if si >= s_len or s[si] == '/':
                    return False
                
                matched = s[si] in class_chars
                if negated:
                    matched = not matched
                
                if not matched:
                    return False
                
                si += 1
            elif pi < p_len and p[pi] == '*':
                # '*' matches zero or more non-slash characters
                # Skip consecutive '*' and try matching at each position
                pi += 1
                # Skip all '*' in pattern
                while pi < p_len and p[pi] == '*':
                    pi += 1
                
                # Try matching remaining pattern against remaining string
                while si <= s_len:
                    if match_segment(p[pi:], s[si:]):
                        return True
                    si += 1
                return False
            elif pi < p_len and si < s_len and p[pi] == s[si]:
                pi += 1
                si += 1
            else:
                return False
        
        return si == s_len and pi == p_len
    
    # Split pattern and path into segments
    pattern_segments = pattern.split('/') if pattern else []
    path_segments = path.split('/') if path else []
    
    # Handle '**' in pattern segments
    def match_segments(p_list, s_list):
        """Match pattern segments (including '**') against path segments."""
        if not p_list:
            return not s_list
        
        if p_list[0] == '**':
            # '**' can match zero or more segments
            # Try matching zero segments first
            if match_segments(p_list[1:], s_list):
                return True
            # Try matching one or more segments
            for i in range(len(s_list)):
                if match_segments(p_list[1:], s_list[i+1:]):
                    return True
            return False
        else:
            # Regular segment matching
            if not s_list:
                return False
            # Handle empty pattern segment matching empty path segment
            return match_segment(p_list[0], s_list[0]) and match_segments(p_list[1:], s_list[1:])
    
    return match_segments(pattern_segments, path_segments)