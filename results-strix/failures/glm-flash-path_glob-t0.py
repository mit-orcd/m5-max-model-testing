def match(pattern, path):
    # Helper to match a pattern segment against a path segment
    def segment_match(p_seg, s_seg):
        i = 0
        j = 0
        p_len = len(p_seg)
        s_len = len(s_seg)

        # Stack to handle backtracking for '*' and '**'
        # Each entry is (i, j, star_idx)
        stack = []

        while j < s_len:
            if i < p_len and p_seg[i] == s_seg[j]:
                i += 1
                j += 1
            elif i < p_len and p_seg[i] == '?':
                i += 1
                j += 1
            elif i < p_len and p_seg[i] == '*':
                # Save current state for backtracking
                stack.append((i, j, len(stack)))
                i += 1
            elif i < p_len and p_seg[i] == '[':
                # Handle character class
                class_start = i
                i += 1
                negated = False
                if i < p_len and p_seg[i] == '!':
                    negated = True
                    i += 1
                elif i < p_len and p_seg[i] == ']':
                    # Empty class matches nothing
                    return False
                
                matched = False
                while i < p_len and p_seg[i] != ']':
                    if i + 1 < p_len and p_seg[i+1] == '-':
                        # Handle range
                        start = p_seg[i]
                        end = p_seg[i+2]
                        if start <= end and start <= s_seg[j] <= end:
                            matched = True
                        i += 2
                    else:
                        if p_seg[i] == s_seg[j]:
                            matched = True
                        i += 1
                
                if i >= p_len:
                    return False # Unclosed class
                
                i += 1 # Skip ']'
                if matched == negated:
                    return False
                j += 1
            elif stack:
                # Backtrack: Star matches one more character
                i, j, _ = stack[-1]
                j += 1
            else:
                return False
        
        # Consume remaining '*' or '**'
        while i < p_len and p_seg[i] == '*':
            i += 1

        return i == p_len and j == s_len

    # Process the pattern and path
    p_parts = []
    s_parts = []
    
    # Parse pattern into segments
    i = 0
    p_len = len(pattern)
    while i < p_len:
        if pattern[i] == '\\':
            if i + 1 < p_len:
                p_parts.append(pattern[i+1])
                i += 2
            else:
                p_parts.append('\\')
                i += 1
        elif pattern[i] == '*':
            # Handle ** as a special segment
            if i + 1 < p_len and pattern[i+1] == '*':
                p_parts.append('**')
                i += 2
            else:
                p_parts.append('*')
                i += 1
        else:
            p_parts.append(pattern[i])
            i += 1

    # Parse path into segments
    i = 0
    s_len = len(path)
    while i < s_len:
        if path[i] == '/':
            s_parts.append('')
            i += 1
        else:
            start = i
            while i < s_len and path[i] != '/':
                i += 1
            s_parts.append(path[start:i])

    # Match segments
    p_idx = 0
    s_idx = 0
    p_len = len(p_parts)
    s_len = len(s_parts)

    while p_idx < p_len and s_idx < s_len:
        p_part = p_parts[p_idx]
        s_part = s_parts[s_idx]

        if p_part == '**':
            # Match zero or more segments
            # Try to match the rest of the pattern against the current or next segment
            if p_idx + 1 == p_len:
                # ** at the end matches everything
                return True
            
            # Try to match the next pattern segment with the current path segment
            if segment_match(p_parts[p_idx+1], s_part):
                p_idx += 2
                s_idx += 1
            else:
                # Try to match the next pattern segment with the next path segment
                if s_idx + 1 < s_len:
                    p_idx += 2
                    s_idx += 1
                else:
                    # No more path segments to match, but pattern has more
                    return False
        else:
            if segment_match(p_part, s_part):
                p_idx += 1
                s_idx += 1
            else:
                return False

    # Handle remaining pattern segments (must be all stars)
    while p_idx < p_len:
        if p_parts[p_idx] != '*':
            return False
        p_idx += 1

    return True