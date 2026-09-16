def match(pattern, path):
    def _match(pattern_iter, path_iter):
        try:
            p_char = next(pattern_iter)
        except StopIteration:
            return next(path_iter, None) is None

        if p_char == '*':
            # Check for '**'
            try:
                next_p = next(pattern_iter)
            except StopIteration:
                next_p = None

            if next_p == '*':
                # '**' matches zero or more segments
                # Try matching zero segments
                if _match(pattern_iter, path_iter):
                    return True
                # Try matching one or more segments
                try:
                    _ = next(path_iter)
                    # Ensure we didn't consume a '/' that was meant to be a separator
                    # We need to handle the path as a stream of characters, but **
                    # matches whole segments. So we need to consume until next '/' or end.
                    while True:
                        if _match(pattern_iter, path_iter):
                            return True
                        try:
                            _ = next(path_iter)
                        except StopIteration:
                            break
                        # If we hit a slash in the path, we are at a segment boundary
                        # But the iterator approach consumes char by char.
                        # We need to restructure to handle segments explicitly for '**'
                except StopIteration:
                    pass
                return False
            else:
                # Single '*' matches zero or more chars but not '/'
                # We must find a split point where the rest matches
                # If path is empty, '*' matches empty string
                if not _match(pattern_iter, path_iter):
                    return True
                # Try consuming characters one by one
                while True:
                    try:
                        path_char = next(path_iter)
                        if path_char == '/':
                            break
                    except StopIteration:
                        break
                    if _match(pattern_iter, path_iter):
                        return True
                return False

        elif p_char == '?':
            try:
                p_char = next(path_iter)
                if p_char == '/':
                    return False
            except StopIteration:
                return False
            return _match(pattern_iter, path_iter)

        elif p_char == '[':
            # Parse character class
            end_class = False
            negated = False
            start_index = 0
            # Find the end of the class
            # We need to handle '!' inside
            class_content = []
            while True:
                try:
                    c = next(pattern_iter)
                except StopIteration:
                    return False # Malformed pattern
                if c == ']':
                    end_class = True
                    break
                if c == '\\' and start_index == 0:
                    # If escaped at start, treat as literal
                    class_content.append(c)
                    start_index += 1
                    continue
                if start_index == 0 and c == '!':
                    negated = True
                start_index += 1
                class_content.append(c)
            
            if not end_class:
                return False # No closing bracket

            try:
                path_char = next(path_iter)
                if path_char == '/':
                    return False
            except StopIteration:
                return False

            is_match = False
            for i, c in enumerate(class_content):
                if i == 0 and c == '!' and not negated:
                    # Actually, the '!' must be the first char to negate
                    # Re-evaluating logic for negation
                    pass
            
            # Re-parse for correctness regarding negation
            # Reset and parse properly
            pattern_iter_list = list(pattern_iter) # This is tricky with iterators.
            # Let's restart parsing logic locally or use a different approach.
            # Since we are inside a recursive function with an iterator, we need to be careful.
            # Let's assume the iterator is at the start of '['.
            pass

        # Simplified approach: Convert pattern to a regex-like logic or handle segments explicitly?
        # The prompt forbids glob/fnmatch.
        # Let's rewrite the logic to handle the iterator more robustly.
        # Actually, re-implementing the parser inside the loop is safer.

        # Reset the logic to handle the character classes and escapes correctly
        # We need to peek ahead or handle the iterator state carefully.
        # Let's use a helper function to parse the class and return the class set and the next iterator state?
        # No, iterators are stateful.
        pass

    # The above recursive approach with iterators is complex for '**' and character classes.
    # Let's convert the pattern to a list of tokens or handle it iteratively with index.
    
    p_len = len(pattern)
    p_idx = 0
    path_len = len(path)
    p_idx = 0
    
    # We need to handle '**' which matches segments.
    # Let's split path into segments? No, because '*' can match parts.
    # But '**' matches whole segments.
    
    # Helper to get next char
    def get_char(idx):
        if idx < p_len:
            return pattern[idx]
        return None

    def get_path_char(idx):
        if idx < path_len:
            return path[idx]
        return None

    # We will use an iterative approach with backtracking or a stack
    # Or recursive with explicit indices
    
    def is_match(p_idx, path_idx):
        # Current pattern index: p_idx
        # Current path index: path_idx
        
        while True:
            # Skip if pattern ends
            if p_idx >= p_len:
                return path_idx >= path_len

            p_char = pattern[p_idx]

            # Handle escape
            if p_char == '\\':
                if p_idx + 1 >= p_len:
                    return False
                next_p = pattern[p_idx+1]
                p_idx += 2
                # Match literally
                if path_idx >= path_len:
                    return False
                if path[path_idx] == next_p:
                    path_idx += 1
                    continue
                else:
                    return False

            # Handle character class
            if p_char == '[':
                # Find closing bracket
                class_end = -1
                i = p_idx + 1
                while i < p_len:
                    if pattern[i] == ']':
                        class_end = i
                        break
                    if pattern[i] == '\\' and i + 1 < p_len:
                        i += 2
                        continue
                    i += 1
                
                if class_end == -1:
                    return False
                
                # Determine if negated
                # Check content between '[' and ']'
                # If first char is '!' or '^' (standard glob)
                negated = False
                start_class = p_idx + 1
                if start_class < class_end:
                    if pattern[start_class] == '!':
                        negated = True
                        start_class += 1
                    elif pattern[start_class] == '^':
                        negated = True
                        start_class += 1
                
                # Extract class chars
                class_chars = set()
                i = start_class
                while i < class_end:
                    c = pattern[i]
                    if pattern[i] == '\\' and i + 1 < class_end:
                        c = pattern[i+1]
                        i += 2
                        class_chars.add(c)
                        continue
                    
                    # Check for range
                    if i + 2 < class_end and pattern[i+1] == '-' and pattern[i+2] != ']':
                        # It's a range
                        start_c = c
                        end_c = pattern[i+2]
                        if end_c == '\\' and i + 3 < class_end:
                            end_c = pattern[i+3]
                            i += 3
                        else:
                            i += 3
                        for char_code in range(ord(start_c), ord(end_c) + 1):
                            class_chars.add(chr(char_code))
                        continue
                    else:
                        class_chars.add(c)
                        i += 1
                
                # Match path char
                if path_idx >= path_len:
                    return False
                path_char = path[path_idx]
                if path_char == '/':
                    return False # Class never matches '/'
                
                is_in_class = path_char in class_chars
                if negated:
                    is_in_class = not is_in_class
                
                if is_in_class:
                    path_idx += 1
                    p_idx = class_end + 1
                    continue
                else:
                    return False

            # Handle '*'
            if p_char == '*':
                # Check for '**'
                if p_idx + 1 < p_len and pattern[p_idx+1] == '*':
                    # It is '**'
                    # Skip the second '*'
                    p_idx += 2
                    
                    # Skip any slashes in pattern after '**' to find the next non-slash
                    # Actually, '**' matches zero or more segments.
                    # We need to try matching zero segments first, then one, etc.
                    
                    # Find the next non-slash in pattern after '**'
                    next_p_idx = p_idx
                    while next_p_idx < p_len and pattern[next_p_idx] == '/':
                        next_p_idx += 1
                    
                    # If pattern is done or only slashes
                    if next_p_idx >= p_len:
                        return path_idx >= path_len or path[path_idx:] == ''
                    
                    # Try matching zero segments:
                    # We just jump to next_p_idx and try to match the rest of pattern
                    # But we must ensure we don't consume path slashes incorrectly?
                    # No, '**' consumes segments.
                    # If we match zero segments, we are at the current path position.
                    # We need to check if the rest of the pattern matches from here.
                    # But the rest of the pattern might start with a slash.
                    # If the pattern after '**' starts with '/', we must be at a segment boundary.
                    # If we matched zero segments, we are at a segment boundary (current path_idx).
                    
                    # Let's use recursion for the '**' logic to handle all possibilities
                    # We need to try:
                    # 1. Match zero segments (current path_idx)
                    # 2. Match one segment (consume until '/')
                    # 3. Match two segments...
                    
                    # Optimization: if the rest of pattern starts with '/', we can only match
                    # if we are at a boundary.
                    
                    # Recursive call for zero segments
                    if next_p_idx >= p_len:
                        return path_idx >= path_len
                    if pattern[next_p_idx] == '/' and path_idx < path_len and path[path_idx] == '/':
                        # Pattern expects a slash, path has it.
                        # We need to advance path_idx to skip the slash if we are matching zero segments?
                        # No, '**' matches segments. The slash is a separator.
                        # If pattern is 'a/**/b', and we are at 'a/', the '**' matches 'b' (zero segments between slashes?)
                        # Actually 'a/**/b' implies a slash after **.
                        # Let's assume the pattern parsing handles slashes.
                        pass
                    
                    # We will implement the backtracking loop for '**'
                    # Try matching zero segments
                    if is_match(next_p_idx, path_idx):
                        return True
                    # Try matching one or more segments
                    while path_idx < path_len:
                        if path[path_idx] == '/':
                            # We are at a segment boundary.
                            # Try matching the rest of the pattern from here
                            if is_match(next_p_idx, path_idx):
                                return True
                            # Move to next segment
                            path_idx += 1
                        else:
                            path_idx += 1
                    # If we reached end of path, check if pattern matches (if pattern ends with slashes)
                    # If path ends, we are at a boundary.
                    if is_match(next_p_idx, path_idx):
                        return True
                    return False

                else:
                    # Single '*'
                    # Matches zero or more chars, not '/'
                    # Try matching zero chars
                    if is_match(p_idx + 1, path_idx):
                        return True
                    # Try matching one or more chars
                    while path_idx < path_len:
                        if path[path_idx] == '/':
                            break
                        path_idx += 1
                        if is_match(p_idx + 1, path_idx):
                            return True
                    return False

            # Handle '?'
            if p_char == '?':
                if path_idx >= path_len:
                    return False
                if path[path_idx] == '/':
                    return False
                path_idx += 1
                p_idx += 1
                continue

            # Handle literal
            if path_idx >= path_len:
                return False
            if path[path_idx] != p_char:
                return False
            path_idx += 1
            p_idx += 1
            continue

        return False

    return is_match(0, 0)