```python
def match(pattern, path):
    """
    Returns True if a glob pattern matches a whole path.
    Paths are separated by '/'.
    '?' matches exactly one character but never '/'.
    '*' matches zero or more characters but never '/'.
    A path segment that is exactly '**' matches zero or more whole segments.
    Square brackets are a character class matching one character.
    A backslash escapes the next character so it is treated literally.
    The whole path must match, not a prefix.
    """
    import re
    
    # Build a regex from the glob pattern
    def glob_to_regex(pattern):
        i = 0
        n = len(pattern)
        regex_parts = []
        
        while i < n:
            c = pattern[i]
            
            if c == '\\':
                # Escape: next character is literal
                if i + 1 < n:
                    next_c = pattern[i + 1]
                    # Escape the character for regex
                    if next_c in r'\^$.|?*+()[]{}':
                        regex_parts.append('\\' + next_c)
                    else:
                        regex_parts.append(next_c)
                    i += 2
                else:
                    # Trailing backslash, treat as literal
                    regex_parts.append('\\\\')
                    i += 1
                    
            elif c == '?':
                # Matches exactly one character but never '/'
                regex_parts.append('[^/]')
                i += 1
                
            elif c == '*':
                # Check if this is '**'
                if i + 1 < n and pattern[i + 1] == '*':
                    # '**' matches zero or more whole segments
                    # We need to handle this carefully
                    # '**' matches zero or more segments
                    # This means it can match '', 'a', 'a/b', 'a/b/c', etc.
                    # In regex terms, this is (?:[^/]+/)*[^/]* or similar
                    # But we need to be careful about the context
                    # Let's use a special approach: '**' matches any number of segments
                    # We'll represent it as a special token and handle it in the regex
                    regex_parts.append('**')
                    i += 2
                else:
                    # Single '*' matches zero or more characters but never '/'
                    regex_parts.append('[^/]*')
                    i += 1
                    
            elif c == '[':
                # Character class
                i += 1
                negated = False
                if i < n and pattern[i] == '!':
                    negated = True
                    i += 1
                elif i < n and pattern[i] == '^':
                    # Some glob implementations use ^ for negation, but the spec says [!...]
                    # Let's stick with ! for negation as per the spec
                    pass
                
                class_chars = []
                if i < n and pattern[i] == ']':
                    # First char is ']', treat as literal
                    class_chars.append(']')
                    i += 1
                    
                while i < n and pattern[i] != ']':
                    c2 = pattern[i]
                    if c2 == '\\':
                        if i + 1 < n:
                            next_c = pattern[i + 1]
                            if next_c in r'\^$.|?*+()[]{}':
                                class_chars.append('\\' + next_c)
                            else:
                                class_chars.append(next_c)
                            i += 2
                        else:
                            class_chars.append('\\\\')
                            i += 1
                    elif c2 == '-' and len(class_chars) >= 2 and i + 1 < n and pattern[i + 1] != ']':
                        # Range
                        start = class_chars.pop()
                        end_c = pattern[i + 1]
                        if end_c == '\\':
                            if i + 2 < n:
                                end_c = pattern[i + 2]
                                i += 3
                            else:
                                end_c = '\\'
                                i += 2
                        else:
                            i += 1
                        # Build range
                        if start <= end_c:
                            class_chars.append(f'{start}-{end_c}')
                        else:
                            # Invalid range, treat as literal
                            class_chars.append(start)
                            class_chars.append('-')
                            class_chars.append(end_c)
                    else:
                        class_chars.append(c2)
                        i += 1
                        
                if i < n and pattern[i] == ']':
                    i += 1
                else:
                    # No closing bracket, treat '[' as literal
                    class_chars.insert(0, '[')
                    regex_parts.append('[' + ''.join(class_chars) + ']')
                    continue
                    
                # Build the character class
                # Ensure '/' is not matched
                inner = ''.join(class_chars)
                # Remove '/' from the class if present
                # Actually, we should ensure the class doesn't match '/'
                # We can do this by adding a condition
                if negated:
                    # Negated class: matches any char except those in class and '/'
                    # We need to be careful: [!abc] should match any char except a, b, c, and /
                    regex_parts.append(f'[^/{"_".join(class_chars)}]')
                else:
                    # Non-negated class: matches chars in class, but not '/'
                    # We need to ensure '/' is excluded
                    # If '/' is in the class, we need to exclude it
                    # Let's just build the class and add a negative lookahead for '/'
                    # Actually, simpler: build the class, then add / to the exclusion
                    # For a non-negated class, we want to match any char in the class except '/'
                    # We can do: [class] but with / excluded
                    # Let's just use a simple approach: match the class, and since we're matching one char,
                    # we can check that it's not '/'
                    # In regex, we can do: (?![/])class or just add / to the negation
                    # For simplicity, let's just build the class and rely on the fact that
                    # if / is in the class, it will match /, which we don't want.
                    # So we need to explicitly exclude /.
                    # Let's rebuild: for non-negated, the class should not include /
                    # We'll filter out / from the class chars
                    filtered = [c for c in class_chars if c != '/']
                    if filtered:
                        regex_parts.append('[' + ''.join(filtered) + ']')
                    else:
                        # Empty class, matches nothing
                        regex_parts.append('(?!)')
                        
            else:
                # Literal character
                if c in r'\^$.|?*+()[]{}':
                    regex_parts.append('\\' + c)
                else:
                    regex_parts.append(c)
                i += 1
        
        return regex_parts
    
    parts = glob_to_regex(pattern)
    
    # Now we need to handle the '**' tokens specially
    # Let's rebuild the regex, handling '**' properly
    # '**' matches zero or more whole segments
    # This is tricky because it depends on context
    
    # Alternative approach: split by '/' and handle segment by segment
    # But '**' can span multiple segments
    
    # Let me think about this differently.
    # We can convert the glob to a regex, treating '**' as a special case.
    # '**' in the middle of a path: a/**/b matches a/b, a/x/b, a/x/y/b, etc.
    # '**' at the start: **/b matches b, x/b, x/y/b, etc.
    # '**' at the end: a/** matches a, a/x, a/x/y, etc.
    # '**' alone: ** matches everything.
    
    # The regex for '**' when it's a standalone segment:
    # It matches zero or more segments, so it can be:
    # - Empty (zero segments)
    # - One or more segments separated by /
    # In regex: (?:[^/]+/)*[^/]* but this doesn't quite work for all cases
    
    # Actually, let's think of it as: '**' matches any string that is a valid path segment sequence.
    # A path is a sequence of segments separated by /.
    # '**' matches zero or more segments, so it matches:
    # - Empty string (zero segments)
    # - A single segment: [^/]+
    # - Multiple segments: [^/]+/[^/]+/.../[^/]+
    # So: (?:[^/]+/)*[^/]*  -- but this matches empty, or one or more segments
    # Wait, (?:[^/]+/)*[^/]* matches:
    # - Zero repetitions of [^/]+/, then [^/]* which can be empty -> matches empty
    # - One repetition of [^/]+/, then [^/]* -> matches "a/" which is not a valid path segment
    # Hmm, this is getting complicated.
    
    # Let me try a different approach: use recursion or dynamic programming.
    # Or, let's just build the regex carefully.
    
    # For '**' as a segment:
    # It matches zero or more whole segments.
    # So in the context of a path split by '/', '**' can match:
    # - Nothing (the segment is removed)
    # - One or more segments
    # 
    # In regex, if we have a pattern like a/**/b, we want to match:
    # a/b, a/x/b, a/x/y/b, etc.
    # The regex would be: a(?:/[^/]+)*/b
    # Because: a, then zero or more /segment, then /b
    # Wait, that gives a/b (zero segments in between), a/x/b (one segment), etc.
    # But a/**/b should also match a//b? No, paths don't have empty segments.
    # So a(?:/[^/]+)*/b is correct.
    
    # For **/b: (?:[^/]+/)*b -- matches b, x/b, x/y/b, etc.
    # For a/**: a(?:/[^/]+)* -- matches a, a/x, a/x/y, etc.
    # For **: (?:[^/]+/)*[^/]* -- but this is tricky. Let's say it matches any valid path.
    # Actually, ** alone should match any path, so: .*\ or just .* but we need to be careful.
    
    # Let me implement this by converting the pattern to a regex, handling ** specially.
    
    def glob_to_regex_v2(pattern):
        i = 0
        n = len(pattern)
        regex = ''
        
        while i < n:
            c = pattern[i]
            
            if c == '\\':
                if i + 1 < n:
                    next_c = pattern[i + 1]
                    if next_c in r'\^$.|?*+()[]{}':
                        regex += '\\' + next_c
                    else:
                        regex += next_c
                    i += 2
                else:
                    regex += '\\\\'
                    i += 1
                    
            elif c == '?':
                regex += '[^/]'
                i += 1
                
            elif c == '*':
                if i + 1 < n and pattern[i + 1] == '*':
                    # '**' segment
                    # Look at context: is it at start, end, or middle?
                    # We need to see what's before and after
                    # For now, let's handle it by looking at the surrounding slashes
                    # Actually, let's just emit a special regex for '**'
                    # '**' matches zero or more segments
                    # In the context of a path, this is: (?:[^/]+/)*[^/]*
                    # But we need to be careful about leading/trailing slashes
                    # Let's handle this by checking if the previous char was '/' and if the next char is '/'
                    # This is getting complex. Let me try a different strategy.
                    # 
                    # Strategy: split the pattern by '/', but handle '**' specially.
                    # Actually, let's just use a regex approach where '**' becomes a special group.
                    # 
                    # For simplicity, let's say '**' matches (?:[^/]+/)*[^/]*
                    # But we need to handle the slashes around it.
                    # 
                    # Let me try: if the pattern has a/**/b, the regex should be a(?:/[^/]+)*/b
                    # If **/b, then (?:[^/]+/)*b
                    # If a/**, then a(?:/[^/]+)*
                    # If **, then (?:[^/]+/)*[^/]*
                    # 
                    # To handle this, I'll look at the characters before and after the '**'
                    # 
                    # Let's just emit a placeholder and handle it later, or use a more sophisticated approach.
                    # 
                    # For now, let's use: (?:[^/]+/)*[^/]* for '**'
                    # But this might not work correctly in all contexts.
                    # 
                    # Let me try a different approach: process the pattern segment by segment.
                    # 
                    # Actually, let's just go with a recursive matching approach instead of regex.
                    # This might be cleaner.
                    pass
                else:
                    regex += '[^/]*'
                    i += 1
                    
            elif c == '[':
                i += 1
                negated = False
                if i < n and pattern[i] == '!':
                    negated = True
                    i += 1
                    
                class_chars = []
                if i < n and pattern[i] == ']':
                    class_chars.append(']')
                    i += 1
                    
                while i < n and pattern[i] != ']':
                    c2 = pattern[i]
                    if c2 == '\\':
                        if i + 1 < n:
                            next_c = pattern[i + 1]
                            if next_c in r'\^$.|?*+()[]{}':
                                class_chars.append('\\' + next_c)
                            else:
                                class_chars.append(next_c)
                            i += 2
                        else:
                            class_chars.append('\\\\')
                            i += 1
                    elif c2 == '-' and len(class_chars) >= 2 and i + 1 < n and pattern[i + 1] != ']':
                        start = class_chars.pop()
                        end_c = pattern[i + 1]
                        if end_c == '\\':
                            if i + 2 < n:
                                end_c = pattern[i + 2]
                                i += 3
                            else:
                                end_c = '\\'
                                i += 2
                        else:
                            i += 1
                        if start <= end_c:
                            class_chars.append(f'{start}-{end_c}')
                        else:
                            class_chars.append(start)
                            class_chars.append('-')
                            class_chars.append(end_c)
                    else:
                        class_chars.append(c2)
                        i += 1
                        
                if i < n and pattern[i] == ']':
                    i += 1
                else:
                    class_chars.insert(0, '[')
                    regex += '[' + ''.join(class_chars) + ']'
                    continue
                    
                if negated:
                    # Negated: match any char except those in class and /
                    # We need to build a negated class
                    # [^/chars]
                    # But we need to escape properly
                    inner = ''.join(class_chars)
                    # Escape regex special chars in inner
                    escaped_inner = ''
                    for ch in inner:
                        if ch in r'\^$.|?*+()[]{}':
                            escaped_inner += '\\' + ch
                        else:
                            escaped_inner += ch
                    # Remove duplicates and ensure / is excluded
                    # For negated, we want [^/...]
                    # But if / is in the class, we still want to exclude it
                    # So: [^/] plus the negated class
                    # Actually, [^/abc] means not /, not a, not b, not c
                    # So we just need to put / and the class chars in the negation
                    all_chars = set('/') | set(inner)
                    # Build the negated class
                    neg_class = ''
                    for ch in sorted(all_chars):
                        if ch in r'\^$.|?*+()[]{}':
                            neg_class += '\\' + ch
                        else:
                            neg_class += ch
                    regex += f'[^{neg_class}]'
                else:
                    # Non-negated: match chars in class, but not /
                    filtered = [c for c in class_chars if c != '/']
                    if filtered:
                        inner = ''.join(filtered)
                        # Escape regex special chars
                        escaped_inner = ''
                        for ch in inner:
                            if ch in r'\^$.|?*+()[]{}':
                                escaped_inner += '\\' + ch
                            else:
                                escaped_inner += ch
                        regex += '[' + escaped_inner + ']'
                    else:
                        regex += '(?!.)'
                        
            else:
                if c in r'\^$.|?*+()[]{}':
                    regex += '\\' + c
                else:
                    regex += c
                i += 1
                
        return regex
    
    # The regex approach is getting too complex. Let me use a recursive matching approach.
    
    def _match(pattern, path):
        """
        Recursive matching for glob patterns.
        """
        if not pattern:
            return not path
            
        if not path:
            # Pattern is not empty, path is empty
            # Only match if pattern is all '*' or '**' that can match empty
            # Check if pattern can match empty string
            return _can_match_empty(pattern)
            
        p0 = pattern[0]
        
        if p0 == '\\':
            if len(pattern) < 2:
                return False
            p1 = pattern[1]
            if path[0] == p1:
                return _match(pattern[2:], path[1:])
            else:
                return False
                
        elif p0 == '?':
            if path[0] == '/':
                return False