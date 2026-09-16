def match(pattern, path):
    def is_match(p, s, pi, si):
        while pi < len(p) and si < len(s):
            if p[pi] == '*':
                # Skip consecutive * characters
                while pi + 1 < len(p) and p[pi + 1] == '*':
                    pi += 1
                pi += 1
                # Try to match the rest of the pattern with the rest of the string
                while si < len(s):
                    if is_match(p, s, pi, si):
                        return True
                    if s[si] == '/':
                        break
                    si += 1
                return is_match(p, s, pi, si)
            elif p[pi] == '?':
                if s[si] == '/':
                    return False
                pi += 1
                si += 1
            elif p[pi] == '[':
                # Parse character class
                pi += 1
                negate = False
                if pi < len(p) and p[pi] == '!':
                    negate = True
                    pi += 1
                # Collect characters in the class
                chars = []
                while pi < len(p) and p[pi] != ']':
                    if pi + 1 < len(p) and p[pi + 1] == '-' and pi + 2 < len(p) and p[pi + 2] != ']':
                        # Range
                        start, end = p[pi], p[pi + 2]
                        for c in range(ord(start), ord(end) + 1):
                            chars.append(chr(c))
                        pi += 3
                    else:
                        chars.append(p[pi])
                        pi += 1
                if pi >= len(p):
                    return False
                pi += 1  # Skip ']'
                matched = s[si] in chars
                if negate:
                    matched = not matched
                if not matched:
                    return False
                si += 1
            elif p[pi] == '\\':
                pi += 1
                if pi >= len(p):
                    return False
                if p[pi] != s[si]:
                    return False
                pi += 1
                si += 1
            else:
                if p[pi] != s[si]:
                    return False
                pi += 1
                si += 1
        # Check if we have consumed all pattern characters
        while pi < len(p) and p[pi] == '*':
            pi += 1
        # Check if we have consumed all string characters
        if pi >= len(p) and si >= len(s):
            return True
        # If we have consumed all pattern characters but not all string characters
        if pi >= len(p) and si < len(s):
            # Check if remaining string has any '/' characters
            for i in range(si, len(s)):
                if s[i] == '/':
                    return False
            return True
        # If we have consumed all string characters but not all pattern characters
        if si >= len(s) and pi < len(p):
            # Check if remaining pattern characters are all '*'
            while pi < len(p):
                if p[pi] != '*':
                    return False
                pi += 1
            return True
        return False
    
    # Handle the special case of '**'
    if '**' in pattern:
        # Convert '**' to a form that can be handled by the main logic
        # Split pattern into segments
        parts = pattern.split('/')
        new_pattern = []
        i = 0
        while i < len(parts):
            if parts[i] == '**':
                new_pattern.append('**')
                # Skip consecutive '**' patterns
                while i + 1 < len(parts) and parts[i + 1] == '**':
                    i += 1
            else:
                new_pattern.append(parts[i])
            i += 1
        pattern = '/'.join(new_pattern)
    
    # Handle the case where pattern or path is empty
    if not pattern and not path:
        return True
    if not pattern:
        return False
    if not path:
        # Check if pattern only contains '*' and '**'
        return all(c == '*' or c == '/' for c in pattern)
    
    # Handle special case for '**' matching
    def is_match_star_star(p, s):
        # This is a simplified version that handles the case where the pattern
        # has '**' and we need to match the path
        # We convert the pattern to a more general form first
        if '**' not in p:
            return is_match(p, s, 0, 0)
        # Split pattern by '**'
        segments = p.split('**')
        if len(segments) == 1:
            return is_match(p, s, 0, 0)
        
        # Handle the case where '**' can match any number of path segments
        # This is a more complex case, we'll handle it by converting to a simpler form
        # and using backtracking
        
        def backtrack(pseg, sseg, p_index, s_index):
            if p_index >= len(pseg):
                return s_index >= len(sseg)
            if s_index >= len(sseg):
                # Check if remaining pattern is all '*'
                for i in range(p_index, len(pseg)):
                    if pseg[i] != '*':
                        return False
                return True
            
            if pseg[p_index] == '*':
                # Match zero or more characters in sseg, but never cross '/'
                while s_index <= len(sseg):
                    if backtrack(pseg, sseg, p_index + 1, s_index):
                        return True
                    if s_index < len(sseg) and sseg[s_index] == '/':
                        break
                    s_index += 1
                return False
            elif pseg[p_index] == '?':
                if sseg[s_index] == '/':
                    return False
                return backtrack(pseg, sseg, p_index + 1, s_index + 1)
            else:
                # Match literal
                if pseg[p_index] != sseg[s_index]:
                    return False
                return backtrack(pseg, sseg, p_index + 1, s_index + 1)
        
        return backtrack(pattern.split('/'), path.split('/'), 0, 0)
    
    # Convert pattern to a more manageable form
    # Split pattern by '/' to handle segments properly
    if '**' in pattern:
        # Split the pattern into segments
        pattern_parts = pattern.split('/')
        path_parts = path.split('/')
        
        # Handle '**' specially
        pi = 0
        si = 0
        
        # Skip leading wildcards
        while pi < len(pattern_parts) and pattern_parts[pi] == '':
            pi += 1
        
        # If pattern starts with '**', match it at the beginning
        if pi < len(pattern_parts) and pattern_parts[pi] == '**':
            # '**' at the beginning can match 0 or more segments
            while pi < len(pattern_parts) and pattern_parts[pi] == '**':
                pi += 1
            
            # Now check if the remaining pattern matches the rest of the path
            if pi >= len(pattern_parts):
                return True
            
            # Try to match the rest of the pattern with the path
            # This is a simplified version that matches the whole path
            def match_segments(pattern_parts, path_parts, pi, si):
                if pi >= len(pattern_parts):
                    return si >= len(path_parts)
                if si >= len(path_parts):
                    # Check if all remaining pattern parts are '*'
                    for i in range(pi, len(pattern_parts)):
                        if pattern_parts[i] != '*':
                            return False
                    return True
                
                if pattern_parts[pi] == '**':
                    # '**' can match 0 or more segments
                    # Try matching 0 segments
                    if match_segments(pattern_parts, path_parts, pi + 1, si):
                        return True
                    # Try matching 1 or more segments
                    while si < len(path_parts):
                        if match_segments(pattern_parts, path_parts, pi + 1, si):
                            return True
                        si += 1
                    return False
                elif pattern_parts[pi] == '*':
                    # '*' can match 0 or more characters (but not '/')
                    while si < len(path_parts):
                        if match_segments(pattern_parts, path_parts, pi + 1, si):
                            return True
                        if path_parts[si] == '/':
                            break
                        si += 1
                    return match_segments(pattern_parts, path_parts, pi + 1, si)
                elif pattern_parts[pi] == '?':
                    if si >= len(path_parts) or path_parts[si] == '/':
                        return False
                    return match_segments(pattern_parts, path_parts, pi + 1, si + 1)
                else:
                    # Literal match
                    if si >= len(path_parts):
                        return False
                    if path_parts[si] != pattern_parts[pi]:
                        return False
                    return match_segments(pattern_parts, path_parts, pi + 1, si + 1)
            
            return match_segments(pattern_parts, path_parts, pi, si)
        
        # General case
        return is_match(pattern, path, 0, 0)
    
    return is_match(pattern, path, 0, 0)