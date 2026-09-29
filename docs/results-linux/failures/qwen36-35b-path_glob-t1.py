def match(pattern: str, path: str) -> bool:
    """
    Match a glob pattern against a path.
    
    Rules:
    - '?' matches exactly one character (not '/')
    - '*' matches zero or more characters (not '/')
    - '**' matches zero or more whole path segments
    - '[...]' is a character class matching one character (not '/')
    - '...' escapes the next character
    - The entire path must match
    - Paths are separated by '/'
    """
    if not pattern and not path:
        return True
    if not pattern:
        return False
    
    # Parse the pattern into tokens
    # Tokens can be:
    # - Literal string (sequence of non-special characters)
    # - '?' 
    # - '*'
    # - '**'
    # - Character class '[...]'
    
    def parse_pattern(pat: str):
        tokens = []
        i = 0
        while i < len(pat):
            c = pat[i]
            if c == '\\':
                # Escape next character
                if i + 1 < len(pat):
                    tokens.append(('literal', pat[i+1]))
                    i += 2
                else:
                    tokens.append(('literal', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('question',))
                i += 1
            elif c == '*':
                if i + 1 < len(pat) and pat[i+1] == '*':
                    tokens.append(('starstar',))
                    i += 2
                    # Skip any trailing '/' after '**'
                    if i < len(pat) and pat[i] == '/':
                        i += 1
                else:
                    tokens.append(('star',))
                    i += 1
            elif c == '[':
                # Parse character class
                j = i + 1
                negated = False
                if j < len(pat) and pat[j] == '!':
                    negated = True
                    j += 1
                chars = set()
                # Handle ] as first character in class
                if j < len(pat) and pat[j] == ']':
                    chars.add(']')
                    j += 1
                while j < len(pat) and pat[j] != ']':
                    if j + 2 < len(pat) and pat[j+1] == '-' and pat[j+2] != ']':
                        # Range
                        start_char = pat[j]
                        end_char = pat[j+2]
                        for k in range(ord(start_char), ord(end_char) + 1):
                            chars.add(chr(k))
                        j += 3
                    else:
                        chars.add(pat[j])
                        j += 1
                if j < len(pat):
                    # Include the closing ']'
                    j += 1
                tokens.append(('class', chars, negated))
                i = j
            else:
                # Collect literal characters
                start = i
                while i < len(pat) and pat[i] not in ('\\', '?', '*', '['):
                    i += 1
                tokens.append(('literal', pat[start:i]))
        return tokens
    
    pat_tokens = parse_pattern(pattern)
    path_segments = path.split('/') if path else []
    
    def match_segments(pat_idx: int, seg_idx: int) -> bool:
        """
        Try to match pat_tokens[pat_idx:] against path_segments[seg_idx:]
        """
        while pat_idx < len(pat_tokens):
            token = pat_tokens[pat_idx]
            
            if token[0] == 'literal':
                # Literal token: must match current segment exactly
                lit = token[1]
                if seg_idx >= len(path_segments):
                    return False
                if path_segments[seg_idx] != lit:
                    return False
                seg_idx += 1
                pat_idx += 1
                
            elif token[0] == 'question':
                # '?' matches exactly one character in the segment, not '/'
                if seg_idx >= len(path_segments):
                    return False
                seg = path_segments[seg_idx]
                # '?' matches exactly one character that is not '/'
                if len(seg) != 1 or seg == '/':
                    return False
                seg_idx += 1
                pat_idx += 1
                
            elif token[0] == 'star':
                # '*' matches zero or more characters in a single segment, not '/'
                pat_idx += 1
                # Try matching zero or more characters in current segment
                if seg_idx >= len(path_segments):
                    # If no more segments, '*' can match zero characters
                    return match_segments(pat_idx, seg_idx)
                seg = path_segments[seg_idx]
                # '*' can match any substring of the segment (since it can't cross '/')
                # It matches zero or more characters within the segment
                # So we try matching the rest of the pattern against the rest of the segments
                # with the current segment potentially being partially consumed
                for k in range(len(seg) + 1):
                    # k is how many characters of the segment '*' consumes
                    # The remaining k characters are not consumed, so we need to handle them
                    # Actually, '*' in a segment position matches zero or more non-'/' characters
                    # So it can match the entire segment or a prefix of it
                    # If it matches the entire segment, move to next segment
                    # If it matches a prefix, the remaining part must be matched by subsequent tokens
                    # But subsequent tokens are either literals, ?, classes, etc.
                    # If '*' is followed by something, that something must match the rest of the current segment or next segments
                    
                    # Let's think differently:
                    # '*' matches zero or more characters that are not '/'
                    # So in the context of segments, '*' applies within a single segment
                    
                    # Approach: try all possible splits of the current segment
                    # '*' consumes seg[:k] (0 <= k <= len(seg))
                    # Then the rest of the pattern must match seg[k:] + remaining segments
                    
                    # But this is complex. Let's use a different approach:
                    # We'll try to match the rest of the pattern starting from the current segment
                    # with the constraint that '*' matches within the segment
                    
                    # Actually, let's just try: for each possible number of characters '*' consumes
                    # from the current segment, check if the remaining pattern matches the rest
                    
                    # If k == len(seg), we move to next segment
                    # If k < len(seg), the remaining seg[k:] must be matched by the rest of the pattern
                    
                    # Let's try matching the rest of the pattern against the current segment's remainder + next segments
                    if k < len(seg):
                        # We need to match pat_tokens[pat_idx:] against a "virtual" segment seg[k:] + path_segments[seg_idx+1:]
                        # But our match_segments works on whole segments. 
                        # We need a helper that can handle partial segments.
                        if match_partial(pat_idx, seg[k:], seg_idx + 1):
                            return True
                    else:
                        # k == len(seg), '*' consumed the entire segment
                        if match_segments(pat_idx, seg_idx + 1):
                            return True
                return False
                
            elif token[0] == 'starstar':
                # '**' matches zero or more whole segments
                pat_idx += 1
                # '**' can match zero or more segments
                # Try matching the rest of the pattern against all possible suffixes of segments
                for k in range(seg_idx, len(path_segments) + 1):
                    if match_segments(pat_idx, k):
                        return True
                return False
                
            elif token[0] == 'class':
                # Character class: matches exactly one character in the segment, not '/'
                chars, negated = token[1], token[2]
                if seg_idx >= len(path_segments):
                    return False
                seg = path_segments[seg_idx]
                # Class matches exactly one character
                if len(seg) != 1 or seg == '/':
                    return False
                if negated:
                    if seg in chars:
                        return False
                else:
                    if seg not in chars:
                        return False
                seg_idx += 1
                pat_idx += 1
        
        # All tokens consumed
        return seg_idx == len(path_segments)
    
    def match_partial(pat_idx: int, partial_seg: str, seg_idx: int) -> bool:
        """
        Match pat_tokens[pat_idx:] against partial_seg + path_segments[seg_idx:]
        where partial_seg is a partial segment (remainder after '*' consumed some chars)
        """
        if not pat_idx < len(pat_tokens):
            # No more pattern tokens
            return partial_seg == '' and seg_idx == len(path_segments)
        
        token = pat_tokens[pat_idx]
        
        if token[0] == 'literal':
            lit = token[1]
            if partial_seg == '' and seg_idx < len(path_segments):
                # Need to match literal against next segment
                if path_segments[seg_idx] != lit:
                    return False
                return match_segments(pat_idx + 1, seg_idx + 1)
            elif partial_seg == lit:
                return match_segments(pat_idx + 1, seg_idx)
            else:
                return False
                
        elif token[0] == 'question':
            # '?' matches exactly one character
            if partial_seg and partial_seg[0] != '/':
                return match_partial(pat_idx + 1, partial_seg[1:], seg_idx)
            elif seg_idx < len(path_segments) and len(path_segments[seg_idx]) == 1 and path_segments[seg_idx] != '/':
                return match_segments(pat_idx + 1, seg_idx + 1)
            return False
            
        elif token[0] == 'star':
            # '*' matches zero or more characters in the partial segment
            pat_idx += 1
            # Try all possible splits of partial_seg
            for k in range(len(partial_seg) + 1):
                remaining_partial = partial_seg[k:]
                if k < len(partial_seg):
                    if match_partial(pat_idx, remaining_partial, seg_idx):
                        return True
                else:
                    # '*' consumed all of partial_seg
                    if match_segments(pat_idx, seg_idx):
                        return True
            return False
            
        elif token[0] == 'starstar':
            # '**' matches zero or more whole segments
            pat_idx += 1
            for k in range(seg_idx, len(path_segments) + 1):
                # Try matching rest of pattern against segments starting from k
                # But we also need to account for partial_seg
                # If partial_seg is not empty, we need to handle it
                if partial_seg == '':
                    if match_segments(pat_idx, k):
                        return True
                else:
                    # partial_seg needs to be matched somehow
                    # This is getting complex. Let's handle it differently.
                    pass
            return False
            
        elif token[0] == 'class':
            chars, negated = token[1], token[2]
            if partial_seg and partial_seg[0] != '/':
                ch = partial_seg[0]
                if negated:
                    if ch in chars:
                        return False
                else:
                    if ch not in chars:
                        return False
                return match_partial(pat_idx + 1, partial_seg[1:], seg_idx)
            elif seg_idx < len(path_segments) and len(path_segments[seg_idx]) == 1 and path_segments[seg_idx] != '/':
                ch = path_segments[seg_idx]
                if negated:
                    if ch in chars:
                        return False
                else:
                    if ch not in chars:
                        return False
                return match_segments(pat_idx + 1, seg_idx + 1)
            return False
        
        return False
    
    return match_segments(0, 0)