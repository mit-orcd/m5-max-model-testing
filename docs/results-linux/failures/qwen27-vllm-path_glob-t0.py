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
    
    # Convert glob pattern to regex
    def glob_to_regex(pattern):
        i = 0
        n = len(pattern)
        regex_parts = []
        
        while i < n:
            c = pattern[i]
            
            if c == '\\':
                # Escape next character
                if i + 1 < n:
                    next_c = pattern[i + 1]
                    regex_parts.append(re.escape(next_c))
                    i += 2
                else:
                    regex_parts.append(re.escape(c))
                    i += 1
            elif c == '?':
                # Matches exactly one character but never '/'
                regex_parts.append('[^/]')
                i += 1
            elif c == '*':
                # Check if it's '**'
                if i + 1 < n and pattern[i + 1] == '*':
                    # '**' matches zero or more whole segments
                    # We need to handle this carefully
                    # '**' can match zero or more segments
                    # This is tricky because it spans across '/'
                    # Let's handle it by looking at the context
                    
                    # Check what comes before and after
                    # If pattern starts with '**' or has '/**'
                    # If pattern ends with '**' or has '**/'
                    
                    # For simplicity, let's build the regex for '**'
                    # '**' matches zero or more segments
                    # A segment is [^/]*
                    # So '**' matches (?:[^/]+/)*[^/]* or similar
                    
                    # Actually, let's think about this differently.
                    # '**' as a whole segment means it can match any number of segments.
                    # If the pattern is 'a/**/b', it should match 'a/b', 'a/x/b', 'a/x/y/b', etc.
                    # If the pattern is '**/b', it should match 'b', 'x/b', 'x/y/b', etc.
                    # If the pattern is 'a/**', it should match 'a', 'a/x', 'a/x/y', etc.
                    
                    # The key insight: '**' when it's a complete segment (surrounded by '/' or at start/end)
                    # matches zero or more complete segments.
                    
                    # Let's check if this '**' is a complete segment
                    # It's a complete segment if:
                    # - It's at the start and the next char is '/' or end
                    # - It's at the end and the previous char is '/' or start
                    # - It's in the middle and both neighbors are '/'
                    
                    prev_is_slash = (i == 0) or (pattern[i - 1] == '/')
                    next_is_slash = (i + 2 >= n) or (pattern[i + 2] == '/')
                    
                    if prev_is_slash and next_is_slash:
                        # This is a complete '**' segment
                        # It matches zero or more segments
                        # We need to be careful about the surrounding slashes
                        
                        # If it's at the start: '**/...' -> matches zero or more segments followed by '/'
                        # If it's at the end: '.../**' -> matches zero or more segments
                        # If it's in the middle: '.../**/...' -> matches zero or more segments
                        
                        # Let's handle by consuming the '**' and adding appropriate regex
                        # The '**' itself doesn't include the slashes, so we need to handle them separately
                        
                        # Actually, let's just add a pattern that matches zero or more segments
                        # A segment is [^/]*
                        # Zero or more segments: (?:[^/]+/)*[^/]*
                        # But we need to be careful about the slashes
                        
                        # Let's think: if pattern is 'a/**/b'
                        # The regex should be: a/(?:[^/]+/)*b  -- but this doesn't match 'a/b'
                        # We need: a/(?:[^/]+/)*b  where the (?:[^/]+/)* can be empty
                        # Actually: a/(?:[^/]+/)*b matches 'a/b' (empty), 'a/x/b', 'a/x/y/b'
                        # Wait, 'a/(?:[^/]+/)*b' -- if the group is empty, we get 'a/b'. Yes!
                        # But what about 'a/**' at the end?
                        # Pattern 'a/**' should match 'a', 'a/x', 'a/x/y'
                        # Regex: a/(?:[^/]+/)*[^/]* -- this matches 'a/' (empty after), 'a/x/', etc.
                        # But we want 'a' to match too. So: a(?:/[^/]+)*
                        
                        # Let me reconsider. Let's handle the slashes as part of the pattern parsing.
                        
                        # When we see '**' as a complete segment, we should:
                        # - If preceded by '/', the '/' is already in the pattern
                        # - If followed by '/', the '/' is already in the pattern
                        
                        # For 'a/**/b':
                        # We parse 'a', then '/', then '**', then '/', then 'b'
                        # When we see '**' with prev='/' and next='/', we add a pattern that matches
                        # zero or more segments, and we need to handle the slashes.
                        
                        # Let's add: (?:[^/]+/)* for the middle case, but we need to be careful.
                        
                        # Actually, let's just add a special marker and handle it in post-processing.
                        # Or better, let's build the regex more carefully.
                        
                        # For a complete '**' segment:
                        # - If at start: matches zero or more segments, so (?:[^/]+/)*[^/]*
                        #   But if followed by '/', we need to adjust
                        # - If at end: matches zero or more segments, so (?:/[^/]+)*
                        # - If in middle: matches zero or more segments, so (?:[^/]+/)*
                        
                        # Let's handle it by looking at what's around
                        if i == 0:
                            # At start
                            if i + 2 < n and pattern[i + 2] == '/':
                                # '**/...' -> zero or more segments followed by '/'
                                # The '/' after '**' is part of the pattern
                                # So we add: (?:[^/]+/)*
                                regex_parts.append('(?:[^/]+/)*')
                                i += 2
                            else:
                                # '**' at start, no following '/'
                                # Matches zero or more segments
                                regex_parts.append('(?:[^/]+/)*[^/]*')
                                i += 2
                        elif i + 2 >= n:
                            # At end
                            if i > 0 and pattern[i - 1] == '/':
                                # '.../**' -> zero or more segments
                                # The '/' before '**' is part of the pattern
                                # So we add: (?:[^/]+/)*
                                regex_parts.append('(?:[^/]+/)*')
                                i += 2
                            else:
                                # '**' at end, no preceding '/'
                                # This shouldn't happen if it's a complete segment
                                regex_parts.append('(?:[^/]+/)*[^/]*')
                                i += 2
                        else:
                            # In the middle: '.../**/...'
                            # Both neighbors are '/'
                            # We add: (?:[^/]+/)*
                            regex_parts.append('(?:[^/]+/)*')
                            i += 2
                    else:
                        # Not a complete segment, treat as regular '*'
                        # But wait, if it's not a complete segment, it's just '*'
                        # Actually, the problem says "A path segment that is exactly '**'"
                        # So if it's not a complete segment, it's just two '*' characters
                        # But that's unusual. Let's treat it as '*' for now
                        regex_parts.append('[^/]*')
                        i += 2
                else:
                    # Single '*'
                    regex_parts.append('[^/]*')
                    i += 1
            elif c == '[':
                # Character class
                j = i + 1
                class_content = ''
                negated = False
                if j < n and pattern[j] == '!':
                    negated = True
                    j += 1
                if j < n and pattern[j] == ']':
                    class_content += ']'
                    j += 1
                while j < n and pattern[j] != ']':
                    if pattern[j] == '\\' and j + 1 < n:
                        class_content += pattern[j + 1]
                        j += 2
                    else:
                        class_content += pattern[j]
                        j += 1
                if j < n and pattern[j] == ']':
                    j += 1  # consume the ']'
                
                if negated:
                    regex_parts.append('[^' + class_content + ']')
                else:
                    regex_parts.append('[' + class_content + ']')
                i = j
            elif c == '/':
                regex_parts.append('/')
                i += 1
            else:
                regex_parts.append(re.escape(c))
                i += 1
        
        return ''.join(regex_parts)
    
    regex = glob_to_regex(pattern)
    return re.fullmatch(regex, path) is not None