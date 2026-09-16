def match(pattern, path):
    """
    Returns True if a glob pattern matches a whole path.
    """
    # Split pattern and path into segments by '/'
    # But we need to be careful with '**' which spans multiple segments
    
    # We'll process the pattern and path character by character or segment by segment
    # Actually, let's do a recursive/DP approach on segments
    
    pat_segments = pattern.split('/')
    path_segments = path.split('/')
    
    def match_segments(pat_segs, path_segs):
        """
        Match pattern segments against path segments.
        """
        # Base cases
        if not pat_segs and not path_segs:
            return True
        if not pat_segs:
            return False
        if not path_segs:
            # Only match if all remaining pattern segments are '**'
            return all(seg == '**' for seg in pat_segs)
        
        pat_seg = pat_segs[0]
        path_seg = path_segs[0]
        
        # If pattern segment is '**', it matches zero or more path segments
        if pat_seg == '**':
            # Try matching zero segments: skip '**' in pattern
            if match_segments(pat_segs[1:], path_segs):
                return True
            # Try matching one or more segments: keep '**' and consume one path segment
            if match_segments(pat_segs, path_segs[1:]):
                return True
            return False
        
        # Otherwise, match the current segment literally with glob rules
        if not match_segment(pat_seg, path_seg):
            return False
        
        # Recurse on the rest
        return match_segments(pat_segs[1:], path_segs[1:])
    
    def match_segment(pat, text):
        """
        Match a single pattern segment against a single text segment.
        Handles ?, *, character classes, and backslash escaping.
        Neither pat nor text should contain '/'.
        """
        return _match_glob(pat, text, 0, 0)
    
    def _match_glob(pat, text, pi, ti):
        """
        Recursive glob matching for a single segment.
        pi: current index in pat
        ti: current index in text
        """
        plen = len(pat)
        tlen = len(text)
        
        # Use memoization to avoid exponential time
        # We'll use a cache key based on (pi, ti)
        if (pi, ti) in _match_glob.cache:
            return _match_glob.cache[(pi, ti)]
        
        result = False
        
        while pi < plen and ti < tlen:
            pch = pat[pi]
            
            if pch == '\\':
                # Escaped character: next character is literal
                if pi + 1 >= plen:
                    break  # Invalid escape at end
                pi += 1
                if pat[pi] != text[ti]:
                    break
                pi += 1
                ti += 1
            elif pch == '?':
                # Matches any single character except '/'
                # Since we're in a segment, '/' shouldn't appear in text
                pi += 1
                ti += 1
            elif pch == '*':
                # Matches zero or more characters (not '/')
                # Try matching zero characters first, then more
                # Skip consecutive '*'s
                while pi < plen and pat[pi] == '*':
                    pi += 1
                
                # If '*' is at the end, match everything
                if pi == plen:
                    result = True
                    break
                
                # Try matching the rest of the pattern against each position in text
                for next_ti in range(ti, tlen + 1):
                    if _match_glob(pat, text, pi, next_ti):
                        result = True
                        break
                break
            elif pch == '[':
                # Character class
                # Find the closing ']'
                if pi + 1 >= plen or pat[pi + 1] == '!':
                    # Check for negation
                    neg = False
                    if pat[pi + 1] == '!':
                        neg = True
                        pi += 2  # Skip '[' and '!'
                    else:
                        pi += 1  # Skip '['
                    
                    # Find closing ']'
                    class_start = pi
                    while pi < plen and pat[pi] != ']':
                        pi += 1
                    
                    if pi >= plen:
                        # No closing ']', treat '[' as literal
                        if pat[class_start - 1] != text[ti]:  # This is tricky, let's reconsider
                            break
                        pi = class_start
                        ti += 1
                        continue
                    
                    # Extract the class content
                    class_content = pat[class_start:pi]
                    pi += 1  # Skip ']'
                    
                    # Check if text[ti] matches the class
                    if matches_class(text[ti], class_content, neg):
                        ti += 1
                    else:
                        result = False
                        break
                else:
                    # Normal class without negation check at start
                    pi += 1  # Skip '['
                    class_start = pi
                    while pi < plen and pat[pi] != ']':
                        pi += 1
                    
                    if pi >= plen:
                        break
                    
                    class_content = pat[class_start:pi]
                    pi += 1  # Skip ']'
                    
                    if matches_class(text[ti], class_content, False):
                        ti += 1
                    else:
                        result = False
                        break
            else:
                # Literal character
                if pch != text[ti]:
                    break
                pi += 1
                ti += 1
        
        # Check if both pattern and text are fully consumed
        if pi == plen and ti == tlen:
            result = True
        elif pi == plen and result:
            # Pattern exhausted, but text might not be (handled by * above)
            pass
        
        _match_glob.cache[(pi, ti)] = result
        return result
    
    def matches_class(ch, class_content, neg):
        """
        Check if character ch matches the character class defined by class_content.
        class_content is the string between '[' and ']', possibly with ranges.
        neg: if True, the match is negated.
        """
        matched = False
        i = 0
        clen = len(class_content)
        
        while i < clen:
            if i + 2 < clen and class_content[i + 1] == '-':
                # Range: class_content[i] to class_content[i + 2]
                lower = class_content[i]
                upper = class_content[i + 2]
                if lower <= ch <= upper:
                    matched = True
                    break
                i += 3
            else:
                if class_content[i] == ch:
                    matched = True
                    break
                i += 1
        
        if neg:
            return not matched
        return matched
    
    # Initialize cache
    _match_glob.cache = {}
    
    return match_segments(pat_segments, path_segments)