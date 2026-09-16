def match(pattern, path):
    def parse_pattern(pattern):
        tokens = []
        i = 0
        n = len(pattern)
        while i < n:
            c = pattern[i]
            if c == '\\':
                if i + 1 < n:
                    tokens.append(('literal', pattern[i + 1]))
                    i += 2
                else:
                    tokens.append(('literal', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('single', None))
                i += 1
            elif c == '*':
                # Check for **
                if i + 1 < n and pattern[i + 1] == '*':
                    # Check if this is a standalone ** (surrounded by / or at start/end)
                    # ** matches zero or more segments
                    # Need to determine if ** is a full segment or part of a larger token
                    # In glob, ** only has special meaning when it's a complete segment
                    # But in the pattern, we need to handle it carefully
                    # Let's collect consecutive *'s
                    j = i
                    while j < n and pattern[j] == '*':
                        j += 1
                    count = j - i
                    if count >= 2:
                        tokens.append(('double_star', None))
                        i = j
                    else:
                        tokens.append(('star', None))
                        i = j
                else:
                    tokens.append(('star', None))
                    i += 1
            elif c == '[':
                # Parse character class
                j = i + 1
                if j < n and pattern[j] == '!':
                    negate = True
                    j += 1
                else:
                    negate = False
                
                chars = set()
                # Handle ] at start
                if j < n and pattern[j] == ']':
                    chars.add(']')
                    j += 1
                
                while j < n and pattern[j] != ']':
                    if pattern[j] == '\\':
                        if j + 1 < n:
                            chars.add(pattern[j + 1])
                            j += 2
                        else:
                            chars.add('\\')
                            j += 1
                    elif j + 2 < n and pattern[j + 1] == '-' and pattern[j + 2] != ']':
                        # Range
                        start = pattern[j]
                        end = pattern[j + 2]
                        if start > end:
                            chars.add(start)
                        else:
                            for cp in range(ord(start), ord(end) + 1):
                                chars.add(chr(cp))
                        j += 3
                    else:
                        chars.add(pattern[j])
                        j += 1
                
                if j < n and pattern[j] == ']':
                    j += 1
                
                tokens.append(('class', (chars, negate)))
                i = j
            else:
                tokens.append(('literal', c))
                i += 1
        
        return tokens
    
    def get_segments(s):
        if not s:
            return []
        return s.split('/')
    
    def match_tokens(tokens, segments, ti, si):
        # ti: index into tokens
        # si: index into segments
        
        # Base case: both exhausted
        if ti == len(tokens) and si == len(segments):
            return True
        if ti == len(tokens):
            return False
        
        token = tokens[ti]
        
        if token[0] == 'double_star':
            # ** matches zero or more segments
            # Try matching remaining tokens with si, si+1, si+2, ..., len(segments)
            for next_si in range(si, len(segments) + 1):
                if match_tokens(tokens, segments, ti + 1, next_si):
                    return True
            return False
        
        if si == len(segments):
            return False
        
        segment = segments[si]
        
        if token[0] == 'literal':
            if len(segment) == 0:
                return False
            if segment[0] != token[1]:
                return False
            # Consume one char from segment and one token
            # But we need to handle the rest of the segment too
            # Let me restructure: instead of splitting into segments upfront,
            # work character by character but track segment boundaries
            
            # Actually, let me rethink. The issue is that * and ? work within segments,
            # but ** works across segments. Let me use a different approach.
            pass
    
    # Let me restart with a cleaner approach
    # Convert pattern to tokens, then use a state machine
    
    tokens = parse_pattern(pattern)
    
    def match_char_in_segment(char, token):
        if token[0] == 'literal':
            return char == token[1]
        elif token[0] == 'single':
            return char != '/'
        elif token[0] == 'class':
            chars, negate = token[1]
            if char == '/':
                return False
            in_set = char in chars
            return in_set != negate  # XOR
        else:
            return False
    
    def match_segment(segment, seg_idx, tokens, tok_idx):
        """Match segment against tokens, returning set of possible token indices after match"""
        # This gets complex. Let me use a different strategy.
        pass
    
    # Cleanest approach: recursive with segment awareness
    def helper(ti, path_idx):
        """Match tokens[ti:] against path[path_idx:]"""
        if ti == len(tokens):
            return path_idx == len(path)
        
        if path_idx == len(path):
            # Path exhausted
            # Only ** can match zero segments
            for i in range(ti, len(tokens)):
                if tokens[i][0] != 'double_star':
                    # Check if remaining tokens can match empty
                    # Actually only double_star can match nothing
                    # But multiple double_stars in a row... 
                    # Let me check: if all remaining tokens are double_star, they can match nothing
                    if all(t[0] == 'double_star' for t in tokens[i:]):
                        return True
                    return False
            return True
        
        token = tokens[ti]
        
        if token[0] == 'double_star':
            # ** matches zero or more segments
            # Try each possible split point
            for next_path_idx in range(path_idx, len(path) + 1):
                if helper(ti + 1, next_path_idx):
                    return True
            return False
        
        # For other tokens, we need to match within the current segment
        # Find the end of the current segment
        seg_end = path_idx
        while seg_end < len(path) and path[seg_end] != '/':
            seg_end += 1
        
        # Match tokens against the segment path[path_idx:seg_end]
        # We need to match as many tokens as possible that are not double_star
        # Then continue with the next segment
        
        # Collect consecutive non-double_star tokens
        next_ti = ti
        while next_ti < len(tokens) and tokens[next_ti][0] != 'double_star':
            next_ti += 1
        
        # Now match tokens[ti:next_ti] against path[path_idx:seg_end]
        # This is a sub-problem: match a sequence of single-char tokens against a string
        def match_in_segment(tok_start, tok_end, str_start, str_end):
            """Match tokens[tok_start:tok_end] against path[str_start:str_end] exactly"""
            t_idx = tok_start
            s_idx = str_start
            
            while t_idx < tok_end:
                if s_idx >= str_end:
                    return False
                tok = tokens[t_idx]
                if tok[0] == 'star':
                    # * matches zero or more chars (not /)
                    # Try all possible lengths
                    for k in range(str_end - s_idx + 1):
                        if match_in_segment(t_idx + 1, tok_end, s_idx + k, str_end):
                            return True
                    return False
                else:
                    if not match_char_in_segment(path[s_idx], tok):
                        return False
                    t_idx += 1
                    s_idx += 1
            
            return s_idx == str_end
        
        if match_in_segment(ti, next_ti, path_idx, seg_end):
            # Matched the segment, move to next segment
            if seg_end < len(path):
                # Skip the '/'
                return helper(next_ti, seg_end + 1)
            else:
                # End of path
                return next_ti == len(tokens)
        
        return False
    
    return helper(0, 0)