def match(pattern: str, path: str) -> bool:
    def parse_bracket(s: str, i: int) -> tuple:
        """
        Parse a character class starting at s[i] which should be '['.
        Returns (parsed_class_info, next_index).
        parsed_class_info is a tuple (negated, set_of_chars, ranges, literal_chars)
        Actually, let's return a function or a set.
        """
        # s[i] is '['
        i += 1
        negated = False
        if i < len(s) and s[i] == '!':
            negated = True
            i += 1
        
        chars = set()
        # Handle '-' at start or end as literal
        if i < len(s) and s[i] == ']':
            chars.add(']')
            i += 1
        
        while i < len(s):
            if s[i] == ']':
                i += 1
                break
            if s[i] == '\\':
                # Escaped character
                if i + 1 < len(s):
                    chars.add(s[i+1])
                    i += 2
                else:
                    # Malformed, but treat backslash as literal at end
                    chars.add('\\')
                    i += 1
            elif i + 1 < len(s) and s[i+1] == '-':
                # Range
                low = s[i]
                # Check if '-' is followed by ']'
                if i + 2 < len(s) and s[i+2] == ']':
                    # '-' is literal
                    chars.add(low)
                    chars.add('-')
                    i += 2
                    continue
                if i + 2 < len(s):
                    high = s[i+2]
                    if low > high:
                        # Invalid range, treat as literals
                        chars.add(low)
                        chars.add('-')
                        chars.add(high)
                        i += 3
                    else:
                        for c in range(ord(low), ord(high) + 1):
                            chars.add(chr(c))
                        i += 3
                else:
                    # '-' at end
                    chars.add(low)
                    chars.add('-')
                    i += 2
            else:
                chars.add(s[i])
                i += 1
        
        return (negated, chars, i)

    def match_segment(pat: str, pidx: int, seg: str, sidx: int, seg_len: int) -> bool:
        """
        Match pattern segment pat[pidx:] against segment seg[sidx:].
        Returns True if the rest of the pattern segment matches the rest of the string segment.
        """
        while pidx < len(pat) and sidx < seg_len:
            pc = pat[pidx]
            if pc == '*':
                # '*' matches zero or more chars except '/'
                # Try all possibilities
                for j in range(sidx, seg_len + 1):
                    if match_segment(pat, pidx + 1, seg, j, seg_len):
                        return True
                return False
            elif pc == '?':
                # Matches exactly one char except '/'
                if sidx < seg_len and seg[sidx] != '/':
                    pidx += 1
                    sidx += 1
                else:
                    return False
            elif pc == '[':
                negated, chars, next_pidx = parse_bracket(pat, pidx)
                if sidx >= seg_len:
                    return False
                sc = seg[sidx]
                if sc == '/':
                    return False
                in_class = sc in chars
                matched = (not in_class) if negated else in_class
                if matched:
                    pidx = next_pidx
                    sidx += 1
                else:
                    return False
            elif pc == '\\':
                if pidx + 1 < len(pat):
                    expected = pat[pidx + 1]
                    if sidx < seg_len and seg[sidx] == expected:
                        pidx += 2
                        sidx += 1
                    else:
                        return False
                else:
                    # Backslash at end of pattern
                    if sidx < seg_len and seg[sidx] == '\\':
                        pidx += 1
                        sidx += 1
                    else:
                        return False
            else:
                if sidx < seg_len and seg[sidx] == pc:
                    pidx += 1
                    sidx += 1
                else:
                    return False
        
        # Check if pattern segment is exhausted
        return pidx == len(pat) and sidx == seg_len

    def match_pattern(pat: str, pidx: int, path: str, path_segments: list, seg_idx: int, seg_start: int) -> bool:
        """
        Match pattern starting at pidx against path segments starting at seg_idx.
        seg_start is the start index in path_segments[seg_idx] to begin matching.
        Actually, let's simplify by splitting path into segments once.
        """
        pass

    # Better approach: split path into segments.
    # Split pattern into segments too, handling '**' specially.
    
    # First, split pattern into parts by '/', but keep '**' as separate parts.
    # However, '**' can appear in the middle of a segment like 'a**b'? 
    # The problem says: "A path segment that is exactly '**' matches zero or more whole segments"
    # So '**' is only special if it's the entire segment.
    
    # Let's split both pattern and path by '/'
    path_segs = path.split('/')
    
    # Split pattern into segments
    pat_segs = pattern.split('/')
    
    # Now we need to match pat_segs against path_segs with '**' semantics.
    # This is a classic glob matching problem.
    # We can use dynamic programming or recursion with memoization.
    
    # Let's define a function that matches pat_segs[pi:] against path_segs[si:]
    # with optional partial matching for the first segment if needed?
    # No, each segment must match fully unless '**' is involved.
    
    # Actually, '**' matches zero or more segments. So when we see '**' in pattern,
    # we can try matching the rest of the pattern against the current position,
    # or skip one segment in the path and try again.
    
    def helper(pi: int, si: int) -> bool:
        """
        Match pat_segs[pi:] against path_segs[si:]
        """
        # Base cases
        if pi == len(pat_segs) and si == len(path_segs):
            return True
        if pi == len(pat_segs):
            return False
        
        seg = pat_segs[pi]
        
        if seg == '**':
            # '**' matches zero or more segments
            # Try matching rest of pattern with current si (zero segments matched by **)
            if helper(pi + 1, si):
                return True
            # Try skipping one segment in path and continuing with same '**'
            if si < len(path_segs):
                if helper(pi, si + 1):
                    return True
            return False
        
        # Regular segment
        if si >= len(path_segs):
            return False
        
        return match_segment(seg, 0, path_segs[si], 0, len(path_segs[si]))
    
    # Wait, there's an issue. If pattern has '**' in the middle like 'a/**/b',
    # pat_segs = ['a', '**', 'b']
    # path = 'a/x/y/b', path_segs = ['a', 'x', 'y', 'b']
    # helper(0, 0): seg='a', matches 'a', then helper(1, 1)
    # helper(1, 1): seg='**', try helper(2, 1) (zero segments) or helper(1, 2) (skip x)
    # helper(2, 1): seg='b', path_segs[1]='x', no match
    # helper(1, 2): seg='**', try helper(2, 2) (zero segments) or helper(1, 3) (skip y)
    # helper(2, 2): seg='b', path_segs[2]='y', no match
    # helper(1, 3): seg='**', try helper(2, 3) (zero segments) or helper(1, 4) (skip b)
    # helper(2, 3): seg='b', path_segs[3]='b', match! helper(3, 4) -> True
    
    # This looks correct. But what about leading/trailing '**'?
    # '**/b' -> pat_segs = ['**', 'b']
    # helper(0, 0): seg='**', try helper(1, 0) or helper(0, 1)
    # helper(1, 0): seg='b', matches path_segs[0] if it's 'b'
    # helper(0, 1): try helper(1, 1) or helper(0, 2)
    # This should work.
    
    # What about 'a/**' matching 'a/x/y'?
    # pat_segs = ['a', '**'], path_segs = ['a', 'x', 'y']
    # helper(0, 0): 'a' matches 'a', helper(1, 1)
    # helper(1, 1): seg='**', try helper(2, 1) (pi==len, si=1, len=3, false) or helper(1, 2)
    # helper(1, 2): try helper(2, 2) (pi==len, si=2, len=3, false) or helper(1, 3)
    # helper(1, 3): try helper(2, 3) (pi==len, si=3, len=3, true!) or helper(1, 4)
    # helper(2, 3) returns True. Good.
    
    # What about 'a/**' matching 'a'?
    # path_segs = ['a']
    # helper(0, 0): 'a' matches 'a', helper(1, 1)
    # helper(1, 1): seg='**', try helper(2, 1) (pi==len, si=1, len=1, true!) 
    # Good.
    
    # What about '**' matching empty path?
    # path = '', path_segs = ['']
    # pat_segs = ['**']
    # helper(0, 0): seg='**', try helper(1, 0) (pi==len, si=0, len=1, false) or helper(0, 1)
    # helper(0, 1): seg='**', si=1 >= len=1, try helper(1, 1) (pi==len, si=1, len=1, true!)
    # Hmm, but empty path should probably not match '**'? Or should it?
    # The problem doesn't specify. Let's assume empty path is a valid segment.
    
    # Actually, let me reconsider the splitting. If path is empty, split gives [''].
    # If pattern is '**', it should match any path including empty? Probably yes.
    
    # One more issue: what if a segment contains special chars but also has '/'?
    # The split handles that since '/' is the delimiter.
    
    return helper(0, 0)