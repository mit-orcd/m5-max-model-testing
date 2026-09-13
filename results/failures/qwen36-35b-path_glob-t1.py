def match(pattern, path):
    return _match_pattern(pattern, 0, path, 0)

def _match_pattern(pattern, p_idx, path, s_idx):
    pattern_len = len(pattern)
    path_len = len(path)
    
    while p_idx < pattern_len and s_idx < path_len:
        c = pattern[p_idx]
        
        if c == '\\':
            # Escaped character
            if p_idx + 1 < pattern_len:
                literal = pattern[p_idx + 1]
                if path[s_idx] == literal:
                    p_idx += 2
                    s_idx += 1
                else:
                    return False
            else:
                # Trailing backslash, invalid escape or literal? 
                # Usually treat as literal backslash if no char follows, 
                # but spec says "escapes the next character". 
                # Assuming valid patterns, this case might not happen or treat as literal.
                # Let's treat trailing backslash as matching literal backslash.
                if path[s_idx] == '\\':
                    p_idx += 1
                    s_idx += 1
                else:
                    return False
        elif c == '?':
            # Matches exactly one character, but never '/'
            if path[s_idx] == '/':
                return False
            p_idx += 1
            s_idx += 1
        elif c == '*':
            # Matches zero or more characters, but never '/'
            # Find the next special character or end of pattern
            # We need to check if this is a '**' pattern
            # Check if it's '**'
            if p_idx + 1 < pattern_len and pattern[p_idx + 1] == '*':
                # It's '**'
                # Skip the two asterisks
                p_idx += 2
                # Skip any following separator
                if p_idx < pattern_len and pattern[p_idx] == '/':
                    p_idx += 1
                
                # '**' matches zero or more whole segments
                # Try matching the rest of the pattern at every possible segment boundary
                # including the current position (zero segments)
                
                # Option 1: Match zero segments (skip '**' and the following slash if present)
                # This is handled by trying the rest of the pattern at current s_idx
                if _match_pattern(pattern, p_idx, path, s_idx):
                    return True
                
                # Option 2: Match one or more segments
                # We advance s_idx until we hit a '/' or end of string
                found_slash = False
                temp_s_idx = s_idx
                while temp_s_idx < path_len:
                    if path[temp_s_idx] == '/':
                        found_slash = True
                        temp_s_idx += 1
                        # Try matching the rest of the pattern from this new segment start
                        if _match_pattern(pattern, p_idx, path, temp_s_idx):
                            return True
                    else:
                        temp_s_idx += 1
                return False
            else:
                # Single '*', matches zero or more chars (no '/')
                # Find the next special char in pattern or end
                # We need to find the longest match of '*'
                # Actually, '*' just consumes chars until '/' or end of string
                # Then tries to match the rest of the pattern.
                
                # Find end of current segment (next '/' or end of string)
                end_s_idx = s_idx
                while end_s_idx < path_len and path[end_s_idx] != '/':
                    end_s_idx += 1
                
                # Try matching the rest of the pattern at every position from s_idx to end_s_idx
                temp_s_idx = s_idx
                while temp_s_idx <= end_s_idx:
                    if _match_pattern(pattern, p_idx + 1, path, temp_s_idx):
                        return True
                    temp_s_idx += 1
                return False
        elif c == '[':
            # Character class
            # Find the closing ']'
            close_bracket = pattern.find(']', p_idx + 1)
            if close_bracket == -1:
                return False
            
            # Determine if negated
            is_negated = False
            check_idx = p_idx + 1
            if check_idx < close_bracket and pattern[check_idx] == '!':
                is_negated = True
                check_idx += 1
            
            matched = False
            # Parse the character class
            i = check_idx
            while i < close_bracket:
                char = pattern[i]
                if i + 2 < close_bracket and pattern[i + 1] == '-' and pattern[i + 2] != ']':
                    # Range
                    start_char = char
                    end_char = pattern[i + 2]
                    if start_char <= path[s_idx] <= end_char:
                        matched = True
                    i += 3
                else:
                    if char == path[s_idx]:
                        matched = True
                    i += 1
            
            if is_negated:
                matched = not matched
            
            if not matched:
                return False
            
            p_idx = close_bracket + 1
            s_idx += 1
        elif c == '/':
            if path[s_idx] == '/':
                p_idx += 1
                s_idx += 1
            else:
                return False
        else:
            # Literal character
            if pattern[p_idx] == path[s_idx]:
                p_idx += 1
                s_idx += 1
            else:
                return False
    
    # Check if both pattern and path are fully consumed
    if p_idx < pattern_len:
        return False
    if s_idx < path_len:
        return False
    
    return True