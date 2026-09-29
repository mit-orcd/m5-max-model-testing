```python
def match(pattern: str, path: str) -> bool:
    """
    Returns True if a glob pattern matches a whole path.
    
    - Paths are separated by '/'.
    - '?' matches exactly one character but never '/'.
    - '*' matches zero or more characters but never '/'.
    - '**' matches zero or more whole segments.
    - Square brackets are a character class matching one character:
      '[abc]', ranges like '[a-z]', and '[!...]' for negation; a class never matches '/'.
    - A backslash escapes the next character so it is treated literally.
    - The whole path must match, not a prefix.
    """
    
    def parse_pattern(pattern: str):
        """Parse pattern into tokens."""
        tokens = []
        i = 0
        n = len(pattern)
        while i < n:
            c = pattern[i]
            if c == '\\':
                # Escaped character
                if i + 1 < n:
                    tokens.append(('literal', pattern[i + 1]))
                    i += 2
                else:
                    # Backslash at end, treat as literal backslash
                    tokens.append(('literal', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('?', None))
                i += 1
            elif c == '*':
                # Check if it's '**'
                if i + 1 < n and pattern[i + 1] == '*':
                    tokens.append(('**', None))
                    i += 2
                else:
                    tokens.append(('*', None))
                    i += 1
            elif c == '[':
                # Parse character class
                j = i + 1
                negate = False
                if j < n and (pattern[j] == '!' or pattern[j] == '^'):
                    negate = True
                    j += 1
                
                # Collect characters until closing ]
                chars = []
                # Handle ] as first character
                if j < n and pattern[j] == ']':
                    chars.append(']')
                    j += 1
                
                while j < n:
                    if pattern[j] == '\\':
                        if j + 1 < n:
                            chars.append(pattern[j + 1])
                            j += 2
                        else:
                            chars.append('\\')
                            j += 1
                    elif pattern[j] == ']':
                        break
                    else:
                        chars.append(pattern[j])
                        j += 1
                
                if j >= n:
                    # No closing bracket, treat [ as literal
                    tokens.append(('literal', '['))
                    i += 1
                else:
                    # We have a valid character class from i to j
                    # Parse the characters for ranges
                    char_set = set()
                    k = 0
                    while k < len(chars):
                        if k + 2 < len(chars) and chars[k + 1] == '-':
                            # Range
                            start = chars[k]
                            end = chars[k + 2]
                            for c in range(ord(start), ord(end) + 1):
                                char_set.add(chr(c))
                            k += 3
                        else:
                            char_set.add(chars[k])
                            k += 1
                    
                    tokens.append(('class', (negate, char_set)))
                    i = j + 1
            else:
                tokens.append(('literal', c))
                i += 1
        
        return tokens
    
    def match_segments(pattern_tokens, path_segments, pi, si):
        """
        Recursively match pattern tokens against path segments.
        pi: index in pattern_tokens
        si: index in path_segments
        """
        # Base cases
        if pi == len(pattern_tokens):
            return si == len(path_segments)
        
        token_type, token_data = pattern_tokens[pi]
        
        if token_type == '**':
            # '**' matches zero or more whole segments
            # Try matching zero segments (skip **)
            if match_segments(pattern_tokens, path_segments, pi + 1, si):
                return True
            # Try matching one or more segments
            if si < len(path_segments):
                if match_segments(pattern_tokens, path_segments, pi, si + 1):
                    return True
            return False
        
        if si >= len(path_segments):
            return False
        
        segment = path_segments[si]
        
        if token_type == 'literal':
            if segment == token_data:
                return match_segments(pattern_tokens, path_segments, pi + 1, si + 1)
            return False
        
        if token_type == '?':
            # ? matches exactly one character but never '/'
            # Since we split by '/', the segment doesn't contain '/'
            # ? matches exactly one character
            if len(segment) == 1:
                return match_segments(pattern_tokens, path_segments, pi + 1, si + 1)
            return False
        
        if token_type == '*':
            # * matches zero or more characters but never '/'
            # Need to match * against the current segment
            return match_star(segment, token_type, 0, 0, pattern_tokens, path_segments, pi, si)
        
        if token_type == 'class':
            negate, char_set = token_data
            # Class matches one character
            # But wait, a class matches exactly one character, so the segment must be length 1
            if len(segment) != 1:
                return False
            c = segment[0]
            if c == '/':
                return False
            in_set = c in char_set
            if negate:
                in_set = not in_set
            if in_set:
                return match_segments(pattern_tokens, path_segments, pi + 1, si + 1)
            return False
        
        return False
    
    def match_star(segment, token_type, seg_idx, pat_idx, pattern_tokens, path_segments, pi, si):
        """
        Match '*' token against a segment.
        seg_idx: current position in segment
        pat_idx: not used here, we're matching within a segment
        """
        # '*' matches zero or more characters (not '/')
        # Try matching zero characters (move to next pattern token)
        if match_segments(pattern_tokens, path_segments, pi + 1, si + 1):
            return True
        # Try matching one or more characters
        if seg_idx < len(segment):
            # Consume one character and try again
            if match_star(segment, token_type, seg_idx + 1, pat_idx, pattern_tokens, path_segments, pi, si):
                return True
        return False
    
    # Special case: handle patterns with ** more carefully
    # The above approach has issues with how ** interacts with segment boundaries.
    # Let me rethink this.
    
    # Better approach: work with the full pattern and full path, but respect segment boundaries.
    
    def match_pattern(pattern_str, path_str):
        """Match pattern against path using recursive matching."""
        return _match(pattern_str, 0, path_str, 0)
    
    def _match(p, pi, s, si):
        """
        Match pattern[p:] against path[s:].
        """
        if pi == len(p):
            return si == len(s)
        
        c = p[pi]
        
        if c == '\\':
            # Escaped character
            if pi + 1 < len(p):
                next_c = p[pi + 1]
                if si < len(s) and s[si] == next_c:
                    return _match(p, pi + 2, s, si + 1)
                return False
            else:
                # Backslash at end
                if si < len(s) and s[si] == '\\':
                    return _match(p, pi + 1, s, si + 1)
                return False
        
        if c == '?':
            if si < len(s) and s[si] != '/':
                return _match(p, pi + 1, s, si + 1)
            return False
        
        if c == '*':
            # Check for **
            if pi + 1 < len(p) and p[pi + 1] == '*':
                # ** matches zero or more whole segments
                # Find the rest of the pattern after **
                rest_pat = pi + 2
                # Skip any extra * after **
                while rest_pat < len(p) and p[rest_pat] == '*':
                    rest_pat += 1
                
                # ** can match zero or more segments
                # Try matching ** against zero segments (i.e., skip to rest of pattern)
                # But we need to be at a segment boundary or end
                
                # Case 1: ** matches nothing. The next part of pattern must match from current position.
                # But ** also consumes segment separators implicitly when matching segments.
                
                # Let's think differently. ** matches zero or more complete path segments.
                # So ** effectively matches: "" (empty, no segments) or "/..." (one or more segments preceded by /)
                # But in the pattern, ** appears between separators or at start/end.
                
                # The key insight: ** matches any sequence of characters including '/', 
                # BUT it must align to segment boundaries.
                
                # Simpler: try all possible positions where ** could end, respecting segment boundaries.
                
                # If we're at the start of a segment or after a /, ** can match zero or more segments.
                
                # Let me use a different strategy: split into segments and match segment by segment with ** handling.
                
                # For now, let's implement ** as: it can match any number of characters including /,
                # but the segments on either side must align.
                
                # Try ** matching zero characters:
                if _match(p, rest_pat, s, si):
                    return True
                
                # Try ** matching one or more characters (including /)
                # But we need to ensure segment alignment. This is tricky with the flat string approach.
                
                # Let me reconsider the approach entirely.
                
                # Actually, let me go back to the segment-based approach but fix the ** handling.
                pass
            
            # Single * matches zero or more characters but not /
            # Try matching * with zero characters
            if _match(p, pi + 1, s, si):
                return True
            # Try matching * with one or more characters (not /)
            if si < len(s) and s[si] != '/':
                if _match(p, pi, s, si + 1):
                    return True
            return False
        
        if c == '[':
            # Parse character class
            j = pi + 1
            negate = False
            if j < len(p) and (p[j] == '!' or p[j] == '^'):
                negate = True
                j += 1
            
            # Collect characters until closing ]
            chars = []
            # Handle ] as first character
            if j < len(p) and p[j] == ']':
                chars.append(']')
                j += 1
            
            while j < len(p):
                if p[j] == '\\':
                    if j + 1 < len(p):
                        chars.append(p[j + 1])
                        j += 2
                    else:
                        chars.append('\\')
                        j += 1
                elif p[j] == ']':
                    break
                else:
                    chars.append(p[j])
                    j += 1
            
            if j >= len(p):
                # No closing bracket, [ is literal
                if si < len(s) and s[si] == '[':
                    return _match(p, pi + 1, s, si + 1)
                return False
            
            # Parse the characters for ranges
            char_set = set()
            k = 0
            while k < len(chars):
                if k + 2 < len(chars) and chars[k + 1] == '-':
                    start = chars[k]
                    end = chars[k + 2]
                    for c in range(ord(start), ord(end) + 1):
                        char_set.add(chr(c))
                    k += 3
                else:
                    char_set.add(chars[k])
                    k += 1
            
            if si >= len(s):
                return False
            
            c = s[si]
            if c == '/':
                return False
            
            in_set = c in char_set
            if negate:
                in_set = not in_set
            
            if in_set:
                return _match(p, j + 1, s, si + 1)
            return False
        
        # Literal character
        if si < len(s) and s[si] == c:
            return _match(p, pi + 1, s, si + 1)
        return False
    
    # The flat string approach for ** is complex. Let me use a segment-based approach properly.
    
    def split_path(path):
        """Split path into segments, preserving structure."""
        if not path:
            return []
        return path.split('/')
    
    def split_pattern(pattern):
        """Split pattern into segments, respecting escapes and brackets."""
        segments = []
        current = []
        i = 0
        n = len(pattern)
        while i < n:
            c = pattern[i]
            if c == '\\':
                if i + 1 < n:
                    current.append('\\')
                    current.append(pattern[i + 1])
                    i += 2
                else:
                    current.append('\\')
                    i += 1
            elif c == '[':
                # Find closing bracket
                j = i + 1
                if j < n and (pattern[j] == '!' or pattern[j] == '^'):
                    j += 1
                if j < n and pattern[j] == ']':
                    j += 1
                while j < n:
                    if pattern[j] == '\\':
                        j += 2
                    elif pattern[j] == ']':
                        break
                    else:
                        j += 1
                if j < n:
                    # Include the entire bracket expression
                    current.append(pattern[i:j+1])
                    i = j + 1
                else:
                    current.append('[')
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
    
    def match_segment(pattern_seg, path_seg):
        """Match a single pattern segment against a path segment."""
        return _match_segment(pattern_seg, 0, path_seg, 0)
    
    def _match_segment(p, pi, s, si):
        if pi == len(p):
            return si == len(s)
        
        c = p[pi]
        
        if c == '\\':
            if pi + 1 < len(p):
                next_c = p[pi + 1]
                if si < len(s) and s[si] == next_c:
                    return _match_segment(p, pi + 2, s, si + 1)
                return False
            else:
                if si < len(s) and s[si] == '\\':
                    return _match_segment(p, pi + 1, s, si + 1)
                return False
        
        if c == '?':
            if si < len(s):
                return _match_segment(p, pi + 1, s, si + 1)
            return False
        
        if c == '*':
            # * matches zero or more characters (within a segment, so no /)
            # Try zero characters
            if _match_segment(p, pi + 1, s, si):
                return True
            # Try one or more characters
            if si < len(s):
                if _match_segment(p, pi, s, si + 1):
                    return True
            return False
        
        if c == '[':
            # Parse character class within segment pattern
            j = pi + 1
            negate = False
            if j < len(p) and (p[j] == '!' or p[j] == '^'):
                negate = True
                j += 1
            
            chars = []
            if j < len(p) and p[j] == ']':
                chars.append(']')
                j += 1
            
            while j < len(p):
                if p[j] == '\\':
                    if j + 1 < len(p):
                        chars.append(p[j + 1])
                        j += 2
                    else:
                        chars.append('\\')
                        j += 1
                elif p[j] == ']':
                    break
                else:
                    chars.append(p[j])
                    j += 1
            
            if j >= len(p):
                # No closing bracket, [ is literal
                if si < len(s) and s[si] == '[':
                    return _match_segment(p, pi + 1, s, si + 1)
                return False
            
            char_set = set()
            k = 0
            while k < len(chars):
                if k + 2 < len(chars) and chars[k + 1] == '-':
                    start = chars[k]
                    end = chars[k + 2]
                    for ch in range(ord(start), ord(end) + 1):
                        char_set.add(chr(ch))
                    k += 3
                else:
                    char_set.add(chars[k])
                    k += 1
            
            if si >= len(s):
                return False
            
            c = s[si]
            in_set = c in char_set
            if negate:
                in_set = not in_set
            
            if in_set:
                return _match_segment(p, j + 1, s, si + 1)
            return False
        
        # Literal character
        if si < len(s) and s[si] == c:
            return _match_segment(p, pi + 1, s, si + 1)
        return False
    
    def match_with_globstar(pattern_segs, path_segs, pi, si):
        """Match pattern segments against path segments with ** support."""
        if pi == len(pattern_segs):
            return si == len(path_segs)
        
        pat_seg = pattern_segs