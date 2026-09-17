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
    
    def escape_regex_char(c):
        if c in r'\^$.|?*+()[]{}':
            return '\\' + c
        return c
    
    def build_regex(pattern):
        """Build a regex from the glob pattern."""
        regex_parts = []
        i = 0
        n = len(pattern)
        
        while i < n:
            c = pattern[i]
            
            if c == '\\':
                # Escape next character
                if i + 1 < n:
                    next_c = pattern[i + 1]
                    regex_parts.append(re.escape(next_c))
                    i += 2
                else:
                    # Trailing backslash, treat as literal
                    regex_parts.append(re.escape('\\'))
                    i += 1
            elif c == '?':
                # Matches exactly one character but never '/'
                regex_parts.append(r'[^/]')
                i += 1
            elif c == '*':
                # Check if this is '**' as a whole segment
                # We need to look at the context: is this '**' surrounded by '/' or at start/end?
                # Actually, we need to check if the segment is exactly '**'
                # Let's handle this by looking at the pattern structure
                
                # Check if we have '**'
                if i + 1 < n and pattern[i + 1] == '*':
                    # We have '**'
                    # Check if it's a whole segment: preceded by start or '/', followed by end or '/'
                    before_ok = (i == 0) or (pattern[i - 1] == '/')
                    after_ok = (i + 2 == n) or (pattern[i + 2] == '/')
                    
                    if before_ok and after_ok:
                        # '**' matches zero or more whole segments
                        # This matches: anything that doesn't contain '/' for one segment, 
                        # and can repeat with '/' separators
                        # Pattern: (?:/[^/]+)*  but also can match empty
                        # Actually, '**' as a segment means it can match:
                        # - nothing (zero segments)
                        # - one or more segments separated by '/'
                        # So the regex should be: (?:/[^/]+)*
                        # But we need to be careful about the surrounding slashes.
                        # If pattern is 'a/**/b', the '**' is between two slashes.
                        # The regex for the whole thing would be: a/(?:/[^/]+)*/b
                        # But 'a/**/b' should match 'a/b' (zero segments) and 'a/x/y/b'
                        # So the part corresponding to '**' should match: (zero or more of: /segment)
                        # That is: (?:/[^/]+)*
                        regex_parts.append(r'(?:/[^/]+)*')
                        i += 2
                    else:
                        # Not a whole segment, treat as regular '*' (but two of them)
                        # Actually, if it's not a whole segment, treat each '*' as matching non-'/' chars
                        # But the spec says '*' matches zero or more characters but never '/'
                        # So '**' that's not a whole segment would just be two '*' in a row,
                        # which is equivalent to one '*'
                        regex_parts.append(r'[^/]*')
                        i += 2
                else:
                    # Single '*'
                    regex_parts.append(r'[^/]*')
                    i += 1
            elif c == '[':
                # Character class
                # Find the closing ']'
                j = i + 1
                negate = False
                if j < n and pattern[j] == '!':
                    negate = True
                    j += 1
                
                # Find the closing bracket
                # The closing bracket is the first ']' that isn't escaped
                # But we also need to handle the case where ']' is the first character
                # In glob, '[]abc]' means the class contains ']', 'a', 'b', 'c'
                
                class_chars = []
                found_close = False
                k = j
                
                while k < n:
                    if pattern[k] == '\\' and k + 1 < n:
                        # Escaped character
                        class_chars.append(re.escape(pattern[k + 1]))
                        k += 2
                    elif pattern[k] == ']':
                        found_close = True
                        k += 1
                        break
                    else:
                        # Check for range
                        if k + 2 < n and pattern[k + 1] == '-' and pattern[k + 2] != ']':
                            # Range
                            start_c = pattern[k]
                            end_c = pattern[k + 2]
                            class_chars.append(f'\\[{re.escape(start_c)}-{re.escape(end_c)}]')
                            # Wait, this is getting complicated. Let me rebuild this.
                            # Actually, let me collect the raw class content and build the regex class properly.
                            pass
                        k += 1
                
                if not found_close:
                    # No closing bracket, treat '[' as literal
                    regex_parts.append(re.escape('['))
                    i += 1
                else:
                    # Build the character class regex
                    # Let's re-extract the class content properly
                    inner_start = i + 1
                    if negate:
                        inner_start += 1
                    
                    # Collect the inner content
                    inner = []
                    k = inner_start
                    while k < n and pattern[k] != ']':
                        if pattern[k] == '\\' and k + 1 < n:
                            inner.append(pattern[k + 1])
                            k += 2
                        else:
                            inner.append(pattern[k])
                            k += 1
                    
                    # Now build the regex character class
                    class_regex = ''
                    if negate:
                        class_regex += '^'
                    
                    m = 0
                    while m < len(inner):
                        ch = inner[m]
                        if ch == '-' and m > 0 and m + 1 < len(inner):
                            # Check if it's a range
                            # A range is: char - char
                            # But we need to be careful: the '-' should be between two characters
                            # and not at the start or end
                            if m > 0 and m < len(inner) - 1:
                                start_c = inner[m - 1]
                                end_c = inner[m + 1]
                                # Remove the previous character from class_regex since we'll add the range
                                class_regex = class_regex[:-len(re.escape(start_c))] if class_regex.endswith(re.escape(start_c)) else class_regex
                                # Actually, this is getting messy. Let me use a different approach.
                                # Let me just build it character by character, handling ranges.
                                pass
                        m += 1
                    
                    # Let me redo the character class building more carefully
                    class_regex = ''
                    if negate:
                        class_regex = '[^'
                    else:
                        class_regex = '['
                    
                    m = 0
                    while m < len(inner):
                        ch = inner[m]
                        if ch == '-' and m > 0 and m + 1 < len(inner):
                            # Potential range
                            start_c = inner[m - 1]
                            end_c = inner[m + 1]
                            # Remove the last added char (start_c) from class_regex
                            if class_regex.endswith(re.escape(start_c)):
                                class_regex = class_regex[:-len(re.escape(start_c))]
                            class_regex += re.escape(start_c) + '-' + re.escape(end_c)
                            m += 2  # Skip the '-' and end_c
                        else:
                            class_regex += re.escape(ch)
                            m += 1
                    
                    class_regex += ']'
                    regex_parts.append(class_regex)
                    i = k + 1  # Skip past the ']'
            else:
                # Literal character
                regex_parts.append(re.escape(c))
                i += 1
        
        return ''.join(regex_parts)
    
    # Build the regex and match
    try:
        regex = build_regex(pattern)
        # Anchor the regex to match the whole string
        full_regex = '^' + regex + '$'
        return re.match(full_regex, path) is not None
    except re.error:
        return False