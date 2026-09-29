def match(pattern, path):
    def parse_segments(s):
        """Split string by '/' into segments, handling escapes."""
        segments = []
        current = []
        i = 0
        while i < len(s):
            c = s[i]
            if c == '\\':
                if i + 1 < len(s):
                    current.append(s[i + 1])
                    i += 2
                else:
                    # Trailing backslash, treat as literal
                    current.append(c)
                    i += 1
            elif c == '/':
                segments.append(''.join(current))
                current = []
                i += 1
            else:
                current.append(c)
                i += 1
        segments.append(''.join(current))
        return segments

    def match_segment(seg_pattern, seg_text):
        """Match a single segment pattern against a single segment text.
        Returns True if they match exactly.
        """
        return match_chars(seg_pattern, seg_text, 0, 0)

    def match_chars(pat, txt, pi, ti):
        """Recursive matcher for characters within a segment."""
        if pi == len(pat):
            return ti == len(txt)
        
        pc = pat[pi]
        
        if pc == '?':
            if ti >= len(txt):
                return False
            # '?' matches any char except '/' (but '/' shouldn't appear in segment)
            return match_chars(pat, txt, pi + 1, ti + 1)
        
        if pc == '*':
            # '*' matches zero or more chars except '/'
            # Try matching zero chars, then one, then two, etc.
            # We need to ensure no '/' is matched, but segments don't have '/'
            for k in range(ti, len(txt) + 1):
                if match_chars(pat, txt, pi + 1, k):
                    return True
            return False
        
        if pc == '[':
            # Parse character class
            # Find closing bracket, handling escapes
            i = pi + 1
            negated = False
            if i < len(pat) and pat[i] == '!':
                negated = True
                i += 1
            
            chars = set()
            ranges = []
            # Parse until unescaped ']'
            first = True
            while i < len(pat):
                if pat[i] == ']' and not first:
                    break
                if pat[i] == '\\':
                    if i + 1 < len(pat):
                        chars.add(pat[i + 1])
                        i += 2
                        first = False
                        continue
                    else:
                        chars.add('\\')
                        i += 1
                        first = False
                        continue
                if i + 2 < len(pat) and pat[i + 1] == '-' and pat[i + 2] != ']':
                    # Range
                    start_char = pat[i]
                    end_char = pat[i + 2]
                    # Check for escaped start or end
                    # Actually, in standard glob, ranges like a-z where a and z are literal
                    # But we need to handle if start or end was preceded by backslash
                    # This is getting complex. Let's re-parse more carefully.
                    pass
                
                # Let me redo the parsing logic properly
                # The above approach is flawed. Let me restart the bracket parsing.
                break
            return False  # Placeholder
        
        # Literal character (could be escaped in original pattern, but we handled escapes in parse_segments)
        # Wait, parse_segments only handles backslash for the whole segment. 
        # But inside brackets, backslash has special meaning too.
        # This approach is getting tangled. Let me rewrite with a cleaner recursive parser.
        
        if ti >= len(txt):
            return False
        if txt[ti] != pc:
            return False
        return match_chars(pat, txt, pi + 1, ti + 1)

    # This is getting messy. Let me write a cleaner implementation.
    
    def match_pattern_segment(pattern_seg, text_seg):
        """Match a pattern segment (with wildcards etc) against a text segment."""
        return _match_seg(pattern_seg, text_seg, 0, 0)
    
    def _match_seg(pat, txt, pi, ti):
        """Match pattern chars from pi against text chars from ti."""
        if pi >= len(pat):
            return ti >= len(txt)
        
        c = pat[pi]
        
        if c == '?':
            if ti >= len(txt):
                return False
            return _match_seg(pat, txt, pi + 1, ti + 1)
        
        if c == '*':
            # Match zero or more characters
            # Greedy backtracking
            for k in range(ti, len(txt) + 1):
                if _match_seg(pat, txt, pi + 1, k):
                    return True
            return False
        
        if c == '[':
            # Parse character class
            # Find the matching closing bracket, handling escapes and negation
            i = pi + 1
            negated = False
            if i < len(pat) and pat[i] == '!':
                negated = True
                i += 1
            
            # First char after [ or [! can be ]
            first = True
            chars = set()
            found_close = False
            
            while i < len(pat):
                if pat[i] == ']' and not first:
                    found_close = True
                    break
                if pat[i] == '\\':
                    if i + 1 < len(pat):
                        chars.add(pat[i + 1])
                        i += 2
                        first = False
                        continue
                    else:
                        chars.add('\\')
                        i += 1
                        first = False
                        continue
                
                # Check for range: x-y
                # Look ahead for '-'
                if i + 2 < len(pat) and pat[i + 1] == '-' and pat[i + 2] != ']':
                    start_c = pat[i]
                    end_c = pat[i + 2]
                    # Expand range
                    for code in range(ord(start_c), ord(end_c) + 1):
                        chars.add(chr(code))
                    i += 3
                    first = False
                    continue
                
                chars.add(pat[i])
                i += 1
                first = False
            
            if not found_close:
                # Malformed, treat '[' as literal? Or fail?
                # For simplicity, treat as literal
                if ti >= len(txt):
                    return False
                if txt[ti] != '[':
                    return False
                return _match_seg(pat, txt, pi + 1, ti + 1)
            
            # Now match the text char
            if ti >= len(txt):
                return False
            matched = txt[ti] in chars
            if negated:
                matched = not matched
            
            if not matched:
                return False
            
            return _match_seg(pat, txt, i + 1, ti + 1)
        
        # Literal character
        if ti >= len(txt):
            return False
        if txt[ti] != c:
            return False
        return _match_seg(pat, txt, pi + 1, ti + 1)
    
    # Parse pattern into segments
    p_segs = parse_segments(pattern)
    t_segs = parse_segments(path)
    
    # Now match segments with '**' logic
    def match_segments(p_segs, t_segs, pi, ti):
        """Match pattern segments from pi against text segments from ti."""
        if pi >= len(p_segs):
            return ti >= len(t_segs)
        
        seg = p_segs[pi]
        
        if seg == '**':
            # '**' matches zero or more segments
            # Try matching zero segments, then one, then two, etc.
            for k in range(ti, len(t_segs) + 1):
                if match_segments(p_segs, t_segs, pi + 1, k):
                    return True
            return False
        
        # Regular segment
        if ti >= len(t_segs):
            return False
        
        if not match_pattern_segment(seg, t_segs[ti]):
            return False
        
        return match_segments(p_segs, t_segs, pi + 1, ti + 1)
    
    return match_segments(p_segs, t_segs, 0, 0)