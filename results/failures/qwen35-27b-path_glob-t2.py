def match(pattern, path):
    # Preprocess pattern to handle backslash escapes
    # Convert pattern to a list of tokens for easier processing
    i = 0
    tokens = []
    while i < len(pattern):
        if pattern[i] == '\\' and i + 1 < len(pattern):
            tokens.append(pattern[i + 1])
            i += 2
        else:
            tokens.append(pattern[i])
            i += 1
    
    # Split path into segments
    path_segments = path.split('/')
    
    # Split pattern into segments, but handle ** specially
    # First, split by '/' but keep track of **
    pattern_segments = []
    current = []
    i = 0
    while i < len(tokens):
        if tokens[i] == '*':
            if i + 1 < len(tokens) and tokens[i + 1] == '*':
                # Found **, treat as special
                pattern_segments.append('**')
                i += 2
            else:
                # Single *
                current.append('*')
                i += 1
        elif tokens[i] == '/':
            if current:
                pattern_segments.append(''.join(current))
                current = []
            i += 1
        else:
            current.append(tokens[i])
            i += 1
    if current:
        pattern_segments.append(''.join(current))
    
    # Now implement matching logic
    # We'll use recursive matching with memoization
    from functools import lru_cache
    
    @lru_cache(maxsize=None)
    def match_segments(p_idx, p_seg, seg_idx, seg):
        # p_idx: current index in pattern_segments
        # p_seg: current pattern segment string
        # seg_idx: current index in path_segments
        # seg: current path segment string
        
        if p_idx == len(pattern_segments):
            return seg_idx == len(path_segments)
        
        if seg_idx == len(path_segments):
            # No more path segments
            # If remaining pattern is only **, it can match zero segments
            remaining = pattern_segments[p_idx:]
            return all(s == '**' for s in remaining)
        
        p_seg = pattern_segments[p_idx]
        seg = path_segments[seg_idx]
        
        if p_seg == '**':
            # ** can match zero or more segments
            # Option 1: match zero segments
            if match_segments(p_idx + 1, None, seg_idx, None):
                return True
            # Option 2: match one or more segments
            # Consume current segment and try again with same **
            if match_segments(p_idx, None, seg_idx + 1, seg):
                return True
            return False
        else:
            # Regular segment matching
            # Use a helper to match the current segment string
            if match_segment_string(p_seg, seg):
                return match_segments(p_idx + 1, None, seg_idx + 1, None)
            return False
    
    def match_segment_string(p_seg, seg):
        # Match a single pattern segment against a single path segment
        # Handle *, ?, [], etc. within the segment
        i, j = 0, 0
        # Use recursion with memoization for segment-level matching
        @lru_cache(maxsize=None)
        def match_str(pi, sj):
            if pi == len(p_seg) and sj == len(seg):
                return True
            if pi == len(p_seg):
                return False
            if sj == len(seg):
                # Only match if remaining pattern is all *
                return all(c == '*' for c in p_seg[pi:])
            
            pc = p_seg[pi]
            sc = seg[sj]
            
            if pc == '*':
                # Match zero or more characters (but not '/')
                # Since we're in a segment, '/' is not present in seg
                # Try skipping this *
                if match_str(pi + 1, sj):
                    return True
                # Try consuming characters
                while sj < len(seg) and match_str(pi, sj + 1):
                    sj += 1
                return match_str(pi, sj + 1) if sj < len(seg) else False
            elif pc == '?':
                if sj < len(seg):
                    return match_str(pi + 1, sj + 1)
                return False
            elif pc == '[':
                # Find the end of the character class
                end = pi + 1
                if end < len(p_seg) and p_seg[end] == '!':
                    end += 1
                elif end < len(p_seg) and p_seg[end] == '^':
                    end += 1
                
                # Find closing ']'
                while end < len(p_seg) and p_seg[end] != ']':
                    end += 1
                
                if end >= len(p_seg):
                    # No closing ']', treat '[' as literal
                    return pc == sc and match_str(pi + 1, sj + 1)
                
                # Extract class content
                neg = pi + 1 < end and p_seg[pi + 1] in ('!', '^')
                class_content = p_seg[pi + 1:end]
                if neg:
                    class_content = class_content[1:]
                
                # Check if sc matches the class
                matches = False
                if sj < len(seg) and sc != '/':
                    # Process ranges and literals
                    k = 0
                    while k < len(class_content):
                        if k + 2 < len(class_content) and class_content[k + 1] == '-':
                            start_char = class_content[k]
                            end_char = class_content[k + 2]
                            if start_char <= sc <= end_char:
                                matches = True
                                k += 3
                                break
                            k += 3
                        else:
                            if sc == class_content[k]:
                                matches = True
                                break
                            k += 1
                
                if matches != neg:  # If negated, invert the match
                    matches = not matches
                
                if matches:
                    return match_str(end + 1, sj + 1)
                return False
            else:
                if pc == sc:
                    return match_str(pi + 1, sj + 1)
                return False
        
        return match_str(0, 0)
    
    # Clear the cache for the inner function
    match_segments.cache_clear()
    return match_segments(0, None, 0, None)