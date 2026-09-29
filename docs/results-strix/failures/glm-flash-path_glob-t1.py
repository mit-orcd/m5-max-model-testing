def match(pattern, path):
    # Preprocess the pattern to handle escapes and collapse '**'
    # We iterate through the pattern string and build a new list of segments
    new_pattern = []
    i = 0
    n = len(pattern)

    while i < n:
        char = pattern[i]
        
        if char == '\\':
            # Handle escape sequences
            if i + 1 < n:
                # Escape the next character
                new_pattern.append(('LIT', pattern[i + 1]))
                i += 2
            else:
                # Trailing backslash, matches literally
                new_pattern.append(('LIT', '\\'))
                i += 1
        elif char == '*':
            # Check for '**'
            if i + 1 < n and pattern[i + 1] == '*':
                # Collapse '**' into a single segment matching zero or more segments
                new_pattern.append(('STAR',))
                i += 2
            else:
                # Single '*', matches zero or more characters but not '/'
                new_pattern.append(('STAR',))
                i += 1
        else:
            # Build character class or literal segment
            class_chars = []
            class_negated = False
            in_class = False
            start = i
            
            if char == '[':
                in_class = True
                i += 1
                if i < n and pattern[i] == '!':
                    class_negated = True
                    i += 1
                elif i < n and pattern[i] == ']':
                    # Empty class, matches nothing
                    i += 1
                    break
            
            while i < n:
                c = pattern[i]
                if c == '\\':
                    if i + 1 < n:
                        class_chars.append(('LIT', pattern[i + 1]))
                        i += 2
                    else:
                        class_chars.append(('LIT', '\\'))
                        i += 1
                elif c == ']':
                    if in_class:
                        i += 1
                        break
                    else:
                        # Not a class closing, literal ']'
                        new_pattern.append(('LIT', '['))
                        new_pattern.append(('LIT', ']'))
                        i += 1
                        break
                elif c == '-':
                    if in_class and len(class_chars) > 0:
                        # Range separator
                        i += 1
                        if i < n and pattern[i] == ']':
                            # Range at end, like '[a-]'
                            class_chars.append(('RANGE', '-', ']'))
                            i += 1
                        elif i < n:
                            # Check if next char is start of range
                            next_char = pattern[i]
                            last_char = class_chars[-1][1]
                            if ord(last_char) <= ord(next_char):
                                class_chars.append(('RANGE', last_char, next_char))
                                i += 1
                            else:
                                # Not a valid range, just literal '-'
                                class_chars.append(('LIT', '-'))
                    else:
                        # Literal '-'
                        class_chars.append(('LIT', '-'))
                        i += 1
                elif c == '[':
                    # Nested class
                    # We need to consume this '[' to avoid treating it as a class start
                    # recursively, but we must process the rest of the nested class
                    # as part of the current class.
                    class_chars.append(('LIT', '['))
                    i += 1
                    # Process the nested class
                    # Find the matching ']'
                    depth = 1
                    start_nested = i
                    while i < n and depth > 0:
                        if pattern[i] == '[':
                            depth += 1
                        elif pattern[i] == ']':
                            depth -= 1
                        i += 1
                    if depth == 0:
                        # Found closing bracket
                        class_chars.append(('LIT', pattern[start_nested:i-1]))
                    else:
                        # Unmatched '[', treat as literal
                        class_chars.append(('LIT', pattern[start_nested:]))
                        break
                else:
                    class_chars.append(('LIT', c))
                    i += 1
            
            if in_class:
                new_pattern.append(('CLASS', class_negated, class_chars))
            else:
                if len(class_chars) == 1:
                    new_pattern.append(('LIT', class_chars[0][1]))
                else:
                    new_pattern.append(('CLASS', False, class_chars))

    # Convert pattern segments to a tuple for hashing
    pattern_segments = tuple(new_pattern)

    # Use memoization to cache results of recursive matching
    memo = {}

    def match_segment(idx, pat_idx, path_idx):
        state = (idx, pat_idx, path_idx)
        if state in memo:
            return memo[state]
        
        # Base case: if we are at the end of the path
        if idx == len(path):
            # For a match, we must also be at the end of the pattern
            # (Unless the last pattern segment is a STAR, which matches empty)
            # However, STAR is handled as a segment.
            # If we are here, we are at end of path.
            # If pat_idx is also end, match is True.
            # If not end, it's False.
            result = (pat_idx == len(pat_segments))
            memo[state] = result
            return result
        
        # If we are past the end of the pattern
        if pat_idx == len(pat_segments):
            # No more segments to match, but path remains. Fail.
            memo[state] = False
            return False

        pat_type, *pat_args = pat_segments[pat_idx]
        char = path[idx]
        
        result = False
        
        if pat_type == 'LIT':
            if char == pat_args[0]:
                result = match_segment(idx + 1, pat_idx + 1, path_idx)
        elif pat_type == 'STAR':
            # STAR matches zero or more characters (excluding '/')
            # We try to match as many as possible (greedy), but backtrack if needed.
            # To simplify, we try to match one character if possible, then recurse.
            # If that fails, we match zero characters.
            
            # Try to match one character
            if char != '/':
                if match_segment(idx + 1, pat_idx, path_idx):
                    result = True
            
            # If that failed, match zero characters
            if not result:
                result = match_segment(idx, pat_idx + 1, path_idx)
        
        elif pat_type == 'CLASS':
            in_class = pat_args[0] # negated
            class_items = pat_args[1]
            
            matched = False
            
            # Check if char matches any item in the class
            for item in class_items:
                if item[0] == 'LIT':
                    if char == item[1]:
                        matched = True
                        break
                elif item[0] == 'RANGE':
                    start_char = item[1]
                    end_char = item[2]
                    if start_char <= char <= end_char:
                        matched = True
                        break
            
            if matched:
                result = not in_class
            else:
                result = in_class
            
            if result:
                result = match_segment(idx + 1, pat_idx + 1, path_idx)

        memo[state] = result
        return result

    # The main matching logic
    # path_segments is just the split string. We pass the original string and index.
    # The '*' segments in our pattern need to handle the '/' logic.
    # The recursive function `match_segment` handles the logic.
    return match_segment(0, 0, path)