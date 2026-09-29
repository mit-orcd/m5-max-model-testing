def match(pattern, path):
    import re
    
    def escape_literal(s):
        return s
    
    # Convert glob pattern to regex
    i = 0
    n = len(pattern)
    regex_parts = []
    
    while i < n:
        c = pattern[i]
        
        if c == '\\':
            if i + 1 < n:
                regex_parts.append(re.escape(pattern[i + 1]))
                i += 2
            else:
                regex_parts.append(re.escape(c))
                i += 1
        elif c == '?':
            regex_parts.append('[^/]')
            i += 1
        elif c == '*':
            # Check if it's '**'
            if i + 1 < n and pattern[i + 1] == '*':
                # '**' matches zero or more whole segments
                # It can match empty, or one or more segments each followed by /
                # So: (?:/[^/]+)*  but we need to handle the slash properly
                # '**' at a segment boundary: matches zero or more complete segments
                # If preceded by '/', then '**' means: /(?:[^/]+/)*[^/]*  ... hmm
                # Let's think: 'a/**/b' should match 'a/b', 'a/x/b', 'a/x/y/b'
                # So '**' between two slashes matches: (?:/[^/]+)*
                # But '**' can also be at the start or end.
                # Let's handle it by looking at context.
                # Actually, let's just emit a pattern that matches zero or more segments.
                # A segment is [^/]+, and segments are separated by /.
                # '**' matches: (?:[^/]+/)*[^/]*  ... no, that's not quite right.
                # 
                # Better approach: '**' should match any number of complete path segments.
                # If the pattern has '/**/', it means zero or more segments in between.
                # Let's emit: (?:/[^/]+)*  for the middle case.
                # But we need to be careful about leading/trailing slashes.
                #
                # Let me think differently. Let's process the pattern character by character
                # and when we see '**', we emit a special regex that matches zero or more segments.
                # The key insight: '**' matches zero or more complete segments.
                # In regex terms, if we're at a segment boundary, '**' can be:
                #   - empty (zero segments)
                #   - one or more segments, each being [^/]+, separated by /
                #
                # Let's just emit: (?:[^/]+/)*  but that has a trailing slash issue.
                # Actually, let's emit: (?:/[^/]+)*  and let the surrounding slashes handle it.
                # Hmm, this is tricky. Let me use a different approach.
                #
                # I'll emit a regex fragment for '**' that matches zero or more complete segments.
                # The fragment is: (?:[^/]+/)*[^/]*  ... no.
                #
                # Let me just use: (?:/[^/]+)*  and trust that the surrounding pattern structure handles it.
                # Actually, for 'a/**/b', the pattern would be: a/ (?:/[^/]+)* /b ... that's not right either.
                #
                # Let me reconsider. I'll build the regex more carefully.
                
                # Emit a pattern that matches zero or more complete segments
                # Each segment is [^/]+, and they're separated by /
                # So zero segments = empty, one segment = [^/]+, two = [^/]+/[^/]+, etc.
                # Regex: (?:[^/]+/)*[^/]*  ... no, that allows trailing slash.
                # Better: (?:[^/]+/)*  matches zero or more segments each followed by /
                # But then we need to handle the case where '**' is followed by something.
                #
                # Let me just emit: (?:[^/]+/)*  and see.
                regex_parts.append('(?:[^/]+/)*')
                i += 2
            else:
                # Single '*' matches zero or more non-slash characters
                regex_parts.append('[^/]*')
                i += 1
        elif c == '[':
            # Find the closing bracket
            j = i + 1
            if j < n and pattern[j] == '!':
                j += 1
            if j < n and pattern[j] == ']':
                j += 1
            while j < n and pattern[j] != ']':
                j += 1
            
            if j < n and pattern[j] == ']':
                # Extract the class content
                content = pattern[i + 1:j]
                negated = content.startswith('!')
                if negated:
                    content = content[1:]
                
                # Escape backslashes in the content
                # Handle ranges and literals
                # For the regex, we need to properly escape characters
                # But ranges like a-z should be preserved
                
                # Build the regex class content
                class_content = ''
                k = 0
                m = len(content)
                while k < m:
                    if content[k] == '\\' and k + 1 < m:
                        class_content += re.escape(content[k + 1])
                        k += 2
                    else:
                        # Check if this is part of a range
                        if k + 2 < m and content[k + 1] == '-' and content[k + 2] != ']':
                            # It's a range
                            class_content += re.escape(content[k]) + '-' + re.escape(content[k + 2])
                            k += 3
                        else:
                            class_content += re.escape(content[k])
                            k += 1
                
                if negated:
                    regex_parts.append('[^' + class_content + ']')
                else:
                    regex_parts.append('[' + class_content + ']')
                i = j + 1
            else:
                # No closing bracket, treat '[' as literal
                regex_parts.append(re.escape(c))
                i += 1
        else:
            regex_parts.append(re.escape(c))
            i += 1
    
    regex = ''.join(regex_parts)
    
    try:
        compiled = re.compile(regex)
        return compiled.fullmatch(path) is not None
    except re.error:
        return False