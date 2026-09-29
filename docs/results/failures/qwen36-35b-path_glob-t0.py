def match(pattern, path):
    def _match(p_idx, path_idx):
        # If we've consumed the entire pattern, check if path is also fully consumed
        if p_idx == len(pattern):
            return path_idx == len(path)
        
        p_char = pattern[p_idx]
        
        # Handle escape character
        if p_char == '\\':
            if p_idx + 1 < len(pattern):
                literal = pattern[p_idx + 1]
                if path_idx < len(path) and path[path_idx] == literal:
                    return _match(p_idx + 2, path_idx + 1)
                else:
                    return False
            else:
                # Trailing backslash, treat as literal? Or invalid? 
                # Usually trailing backslash is invalid or treated as literal.
                # Let's treat it as literal backslash.
                if path_idx < len(path) and path[path_idx] == '\\':
                    return _match(p_idx + 1, path_idx + 1)
                return False
        
        # Handle '**' (double star)
        if p_char == '*' and p_idx + 1 < len(pattern) and pattern[p_idx + 1] == '*':
            # Check if it's a standalone segment '**'
            # It must be preceded by '/' or be the start, and followed by '/' or end
            # Actually, the spec says "A path segment that is exactly '**' matches zero or more whole segments"
            # So we need to check boundaries.
            
            # Check if this is a segment boundary
            # Previous char must be '/' or start of string
            prev_is_sep = (p_idx == 0) or (pattern[p_idx - 1] == '/')
            
            # Next char must be '/' or end of string
            next_is_sep = (p_idx + 2 == len(pattern)) or (pattern[p_idx + 2] == '/')
            
            if prev_is_sep and next_is_sep:
                # This is a '**' segment
                # It matches zero or more whole segments.
                # We need to try matching the rest of the pattern against the current path position,
                # and also skip the current path segment and try again.
                
                # Skip the '**' and the following '/' in pattern
                next_p_idx = p_idx + 2
                if next_p_idx < len(pattern) and pattern[next_p_idx] == '/':
                    next_p_idx += 1
                
                # Try matching the rest of the pattern starting from next_p_idx
                # against the current path position (zero segments consumed)
                if _match(next_p_idx, path_idx):
                    return True
                
                # Try consuming one or more path segments
                # Find the next '/' in path
                next_slash = path.find('/', path_idx)
                if next_slash == -1:
                    # No more slashes, consume rest of path
                    if _match(next_p_idx, len(path)):
                        return True
                else:
                    # Consume up to the next slash
                    if _match(next_p_idx, next_slash + 1):
                        return True
                
                return False
            else:
                # It's a single '*' or part of a pattern like 'a*b'
                # Handle as single '*'
                p_char = '*'
                # Fall through to single '*' handling
        
        # Handle single '*'
        if p_char == '*':
            # Matches zero or more characters, but never '/'
            # Try matching zero characters, then one, etc.
            for i in range(path_idx, len(path) + 1):
                # Check if we've hit a '/'
                if i < len(path) and path[i] == '/':
                    break
                if _match(p_idx + 1, i):
                    return True
            return False
        
        # Handle '?'
        if p_char == '?':
            if path_idx < len(path) and path[path_idx] != '/':
                return _match(p_idx + 1, path_idx + 1)
            return False
        
        # Handle character class [...]
        if p_char == '[':
            # Find the closing bracket
            close_bracket = pattern.find(']', p_idx + 1)
            if close_bracket == -1:
                return False
            
            # Determine if negated
            negated = False
            start = p_idx + 1
            if start < close_bracket and pattern[start] == '!':
                negated = True
                start += 1
            
            # Parse the character class
            matched_char = False
            i = start
            while i < close_bracket:
                if i + 2 < close_bracket and pattern[i + 1] == '-':
                    # Range
                    start_char = pattern[i]
                    end_char = pattern[i + 2]
                    if path_idx < len(path):
                        c = path[path_idx]
                        if start_char <= c <= end_char:
                            matched_char = True
                    i += 3
                else:
                    # Single character
                    if path_idx < len(path) and path[path_idx] == pattern[i]:
                        matched_char = True
                    i += 1
            
            if path_idx < len(path) and path[path_idx] == '/':
                matched_char = False # Class never matches '/'
            
            if negated:
                if not matched_char:
                    return _match(close_bracket + 1, path_idx + 1)
                else:
                    return False
            else:
                if matched_char:
                    return _match(close_bracket + 1, path_idx + 1)
                else:
                    return False
        
        # Literal character
        if path_idx < len(path) and path[path_idx] == p_char:
            return _match(p_idx + 1, path_idx + 1)
        
        return False
    
    return _match(0, 0)