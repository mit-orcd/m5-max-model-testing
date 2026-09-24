import re

def match(pattern, path):
    """
    Returns True if a glob pattern matches a whole path.
    Paths are separated by '/'.
    '?' matches exactly one character but never '/'.
    '*' matches zero or more characters but never '/'.
    A path segment that is exactly '**' matches zero or more whole segments.
    Square brackets are a character class matching one character:
      '[abc]', ranges like '[a-z]', and '[!...]' for negation.
      A class never matches '/'.
    A backslash escapes the next character so it is treated literally.
    The whole path must match, not a prefix.
    """
    # We will parse the pattern into a regex manually to handle ** correctly
    # and other glob features without using fnmatch/glob/pathlib.
    
    regex_parts = []
    i = 0
    n = len(pattern)
    
    while i < n:
        c = pattern[i]
        
        if c == '\\':
            # Escape next character
            if i + 1 < n:
                next_c = pattern[i + 1]
                # Escape it for regex
                regex_parts.append(re.escape(next_c))
                i += 2
            else:
                # Backslash at end, treat as literal backslash
                regex_parts.append(re.escape(c))
                i += 1
        elif c == '?':
            # Matches exactly one character but never '/'
            regex_parts.append('[^/]')
            i += 1
        elif c == '*':
            # Check if this is part of a '**' segment
            # We need to determine if this '*' is a standalone '**' segment
            # A standalone '**' segment is surrounded by '/' or start/end of string.
            
            # Look ahead for another '*'
            if i + 1 < n and pattern[i + 1] == '*':
                # It's '**'
                # Check if it's a whole segment:
                # Previous char is '/' or i == 0
                # Next char after '**' is '/' or i+2 == n
                
                is_start_segment = (i == 0 or pattern[i - 1] == '/')
                # Check end of segment
                j = i + 2
                # Skip any extra stars? Standard glob usually just looks for '**'
                # Let's assume exactly two stars for now, but if there are more, 
                # some implementations treat them as multiple stars.
                # The prompt says "A path segment that is exactly '**'".
                # So we check if the segment consisting of stars is exactly '**'.
                
                # Find the extent of the star sequence
                k = i
                while k < n and pattern[k] == '*':
                    k += 1
                num_stars = k - i
                
                # Determine boundaries of this segment
                # Start boundary: previous char
                # End boundary: char at k
                prev_char = pattern[i - 1] if i > 0 else None
                next_char = pattern[k] if k < n else None
                
                is_whole_segment = (
                    (prev_char is None or prev_char == '/') and
                    (next_char is None or next_char == '/')
                )
                
                if num_stars == 2 and is_whole_segment:
                    # '**' matching zero or more segments
                    # This means it can match:
                    # 1. Nothing (if followed by / which is handled by surrounding logic?)
                    # Actually, it's easier to handle ** in a way that consumes separators.
                    # If we have a/**/b, it matches a/b, a/x/b, etc.
                    # Regex-wise: if ** is at start, end, or middle.
                    
                    # Strategy: 
                    # If ** is at the beginning: matches anything including nothing, but if nothing, it consumes the following /?
                    # Standard behavior: a/**/b -> regex a/(?:[^/]+/)*?b or similar.
                    
                    # Let's look at the context.
                    # If i==0, it's at start.
                    # If k==n, it's at end.
                    
                    # We need to handle the slashes around it.
                    # If it's a middle segment, it's surrounded by /.
                    # e.g., a/**/b. The pattern has a / ** / b.
                    # We want to match a/ ... /b where ... is zero or more segments.
                    # Zero segments means a/b. So the ** and the slashes around it effectively become a single / if it matches nothing?
                    # Or rather, ** matches empty, and the slashes remain? No, a//b is not usually matched.
                    
                    # Common implementation:
                    # ** at start: ^(.*/)?
                    # ** at end: (/.*$)?
                    # ** in middle: /(?:[^/]+/)*
                    
                    # However, we are building a regex for the whole string.
                    # Let's handle the surrounding slashes.
                    
                    # If ** is a whole segment, it is delimited by / or boundaries.
                    # We need to consume the delimiters appropriately.
                    
                    # If i == 0 and k < n and pattern[k] == '/':
                    #   The ** matches zero or more segments followed by a /. 
                    #   Or it matches nothing and the / remains? 
                    #   If ** matches nothing, a/**/b becomes a//b? No, usually a/b.
                    #   So if ** matches nothing, it and the following / are consumed? Or the preceding /?
                    
                    # Let's use a standard approach:
                    # Replace ** with a regex that handles the slash context.
                    
                    # Determine if we should consume adjacent slashes.
                    # If ** is at start: ^(?:.*/)?
                    # If ** is at end: (?:/.*)?$
                    # If ** is in middle: /(?:[^/]+/)*
                    # But this assumes the slashes are part of the pattern.
                    
                    # In the pattern a/**/b, we have tokens a, /, **, /, b.
                    # We process characters sequentially.
                    # When we see **, we need to know if it's at start/end/middle.
                    
                    # Let's handle the slash before and after.
                    
                    # If i > 0 and pattern[i-1] == '/':
                    #   We already added a '/' to regex_parts.
                    #   We need to modify the last added part or handle it here.
                    #   This gets messy.
                    
                    # Alternative: Process ** specially by looking at context.
                    # If we encounter ** as a segment:
                    #   If at start: regex += '(?:.*/)?'  -- but we need to make sure it matches to end or next part?
                    #   Actually, if we replace the segment ** and its surrounding slashes with a regex.
                    
                    # Let's try to replace the sequence of characters forming the ** segment and its delimiters.
                    
                    # Find the full segment including delimiters.
                    start_idx = i
                    end_idx = k
                    
                    # Check left delimiter
                    left_slash_consumed = False
                    if i > 0 and pattern[i-1] == '/':
                        left_slash_consumed = True
                        start_idx = i - 1
                    else:
                        left_slash_consumed = False
                        
                    # Check right delimiter
                    right_slash_consumed = False
                    if k < n and pattern[k] == '/':
                        right_slash_consumed = True
                        end_idx = k + 1
                    else:
                        right_slash_consumed = False
                    
                    # Now we have a segment from start_idx to end_idx which includes the ** and possibly adjacent slashes.
                    # We need to construct a regex for this segment.
                    
                    # Cases:
                    # 1. At start (i==0), no left slash. 
                    #    If right slash consumed: **/ -> (?:.*/)?  (matches nothing or something/)
                    #    If right slash not consumed: ** at end -> (?:.*)? or just .*? 
                    #    Wait, if ** is at end, it matches everything remaining including slashes.
                    #    So ** at end -> .*
                    
                    # 2. At end (k==n), no right slash.
                    #    If left slash consumed: /** -> /.*
                    #    If left slash not consumed: ** at start -> .*
                    
                    # 3. Middle (has both slashes).
                    #    /**/ -> /(?:[^/]+/)* 
                    #    But wait, if ** matches nothing, /**/b becomes /b? 
                    #    Original: a/**/b. If ** matches nothing, we want a/b.
                    #    If we replace /**/ with /(?:[^/]+/)*, then a/**/b -> a/(?:[^/]+/)*b.
                    #    If ** matches nothing, the regex is a/b. Correct.
                    #    If ** matches x/y, regex is a/x/y/b. Correct.
                    
                    # So logic:
                    # If left_slash_consumed and right_slash_consumed:
                    #   regex += '/(?:[^/]+/)*'
                    #   We must remove the previously added '/' from regex_parts.
                    # If left_slash_consumed and not right_slash_consumed:
                    #   regex += '/.*'
                    #   Remove previously added '/'.
                    # If not left_slash_consumed and right_slash_consumed:
                    #   regex += '(?:.*/)?'
                    # If not left_slash_consumed and not right_slash_consumed:
                    #   regex += '.*'
                    
                    # But wait, if left_slash_consumed, we added a '/' to regex_parts earlier.
                    # We need to pop it.
                    
                    if left_slash_consumed:
                        # Remove the last part which should be '/'
                        if regex_parts and regex_parts[-1] == '/':
                            regex_parts.pop()
                        else:
                            # This shouldn't happen if we parsed correctly, but safe guard
                            pass
                            
                    if left_slash_consumed and right_slash_consumed:
                        regex_parts.append('/(?:[^/]+/)*')
                    elif left_slash_consumed:
                        regex_parts.append('/.*')
                    elif right_slash_consumed:
                        regex_parts.append('(?:.*/)?')
                    else:
                        regex_parts.append('.*')
                    
                    i = end_idx
                    continue
                else:
                    # Single '*' or multiple '*' not forming a whole segment
                    # '*' matches zero or more chars except '/'
                    regex_parts.append('[^/]*')
                    i += 1
                    continue
            else:
                # Single '*'
                regex_parts.append('[^/]*')
                i += 1
        elif c == '[':
            # Character class
            j = i + 1
            negated = False
            if j < n and pattern[j] == '!':
                negated = True
                j += 1
            
            # Find closing bracket
            # Escapes inside brackets? Prompt doesn't specify, but usually backslash works.
            # Simple parsing: find next ']' that is not escaped.
            k = j
            chars = []
            while k < n:
                if pattern[k] == '\\':
                    if k + 1 < n:
                        chars.append(pattern[k+1])
                        k += 2
                        continue
                    else:
                        chars.append('\\')
                        k += 1
                        continue
                elif pattern[k] == ']':
                    break
                else:
                    chars.append(pattern[k])
                    k += 1
            
            if k >= n:
                # No closing bracket, treat '[' as literal
                regex_parts.append(re.escape(c))
                i += 1
                continue
            
            # Process chars for range
            # Build regex class
            class_content = ''
            idx = 0
            c_len = len(chars)
            while idx < c_len:
                char = chars[idx]
                # Check for range: char - char
                if idx + 2 < c_len and chars[idx + 1] == '-':
                    start_char = char
                    end_char = chars[idx + 2]
                    # Add range to regex
                    # Escape special regex chars in range
                    def escape_regex_char(ch):
                        if ch in '\\^].-':
                            return '\\' + ch
                        return ch
                    class_content += escape_regex_char(start_char) + '-' + escape_regex_char(end_char)
                    idx += 3
                else:
                    class_content += escape_regex_char(char)
                    idx += 1
            
            if negated:
                regex_parts.append('[^/' + class_content + ']')
            else:
                regex_parts.append('[' + class_content + ']')
            
            i = k + 1
        else:
            # Literal character
            regex_parts.append(re.escape(c))
            i += 1
    
    regex_str = ''.join(regex_parts)
    # Add anchors to ensure whole string match
    regex_str = '^' + regex_str + '$'
    
    try:
        return re.match(regex_str, path) is not None
    except re.error:
        return False