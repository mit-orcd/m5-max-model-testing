def match(pattern, path):
    """
    Returns True if a glob pattern matches a whole path.
    
    Rules:
    - Paths are separated by '/'
    - '?' matches exactly one character but never '/'
    - '*' matches zero or more characters but never '/'
    - '**' matches zero or more whole segments
    - Square brackets are character classes: [abc], [a-z], [!...] for negation
    - A backslash escapes the next character
    - The whole path must match
    """
    # Parse pattern into tokens
    # Tokens: literal chars, '?', '*', '**', character classes
    
    def parse_pattern(pattern):
        tokens = []
        i = 0
        n = len(pattern)
        while i < n:
            c = pattern[i]
            if c == '\\':
                # Escape next character
                if i + 1 < n:
                    tokens.append(('literal', pattern[i+1]))
                    i += 2
                else:
                    # Backslash at end, treat as literal backslash
                    tokens.append(('literal', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('?', None))
                i += 1
            elif c == '*':
                # Check for **
                if i + 1 < n and pattern[i+1] == '*':
                    # Check if this is a full segment **
                    # It's ** if surrounded by / or at start/end
                    # Actually, ** matches zero or more whole segments
                    # We need to handle ** specially
                    # Let's check if it's exactly "**" as a segment
                    # A segment is between / or at boundaries
                    # Find the extent of this **
                    j = i + 2
                    # Skip any additional * (though typically just **)
                    while j < n and pattern[j] == '*':
                        j += 1
                    # Check if this ** forms a complete segment
                    # It does if preceded by / or start, and followed by / or end
                    prev_ok = (i == 0 or pattern[i-1] == '/')
                    next_ok = (j == n or pattern[j] == '/')
                    if prev_ok and next_ok:
                        tokens.append(('**', None))
                        i = j
                    else:
                        # Not a full segment **, treat as single *
                        tokens.append(('*', None))
                        i += 1
                else:
                    tokens.append(('*', None))
                    i += 1
            elif c == '[':
                # Parse character class
                j = i + 1
                if j < n and pattern[j] == '!':
                    negated = True
                    j += 1
                else:
                    negated = False
                
                chars = []
                # Handle ] as first char after [ or [!
                if j < n and pattern[j] == ']':
                    chars.append(']')
                    j += 1
                
                while j < n:
                    if pattern[j] == ']':
                        break
                    if pattern[j] == '\\':
                        if j + 1 < n:
                            chars.append(pattern[j+1])
                            j += 2
                        else:
                            chars.append('\\')
                            j += 1
                    elif j + 2 < n and pattern[j+1] == '-' and pattern[j+2] != ']':
                        # Range
                        start = pattern[j]
                        end = pattern[j+2]
                        for k in range(ord(start), ord(end) + 1):
                            chars.append(chr(k))
                        j += 3
                    else:
                        chars.append(pattern[j])
                        j += 1
                
                if j < n and pattern[j] == ']':
                    j += 1
                else:
                    # No closing bracket, treat [ as literal
                    tokens.append(('literal', '['))
                    i += 1
                    continue
                
                tokens.append(('class', (negated, set(chars))))
                i = j
            else:
                tokens.append(('literal', c))
                i += 1
        
        return tokens
    
    def match_tokens(tokens, ti, path, pi):
        """
        Match tokens[ti:] against path[pi:]
        Returns True if match, False otherwise
        """
        # Base case: no more tokens
        if ti == len(tokens):
            return pi == len(path)
        
        token_type, token_data = tokens[ti]
        
        if token_type == 'literal':
            if pi < len(path) and path[pi] == token_data:
                return match_tokens(tokens, ti + 1, path, pi + 1)
            return False
        
        elif token_type == '?':
            if pi < len(path) and path[pi] != '/':
                return match_tokens(tokens, ti + 1, path, pi + 1)
            return False
        
        elif token_type == '*':
            # * matches zero or more chars but not /
            # Try all possible lengths
            for end in range(pi, len(path) + 1):
                # Check if path[pi:end] contains no /
                if '/' in path[pi:end]:
                    break
                if match_tokens(tokens, ti + 1, path, end):
                    return True
            return False
        
        elif token_type == '**':
            # ** matches zero or more whole segments
            # A segment is a sequence of non-/ chars
            # ** can match:
            # - Zero segments: just skip, but need to handle the / appropriately
            # - One or more segments
            
            # First, try matching with ** consuming zero segments
            # This means we skip the ** and try to match remaining tokens
            # But we need to be careful about the / separators
            
            # Let's think about this differently.
            # ** matches zero or more segments.
            # If ** is at the start, it can match "a/b/c" or ""
            # If ** is in the middle like a/**/b, it matches a/b, a/x/b, a/x/y/b
            
            # The key insight: ** matches zero or more segments, where a segment
            # is a maximal sequence of non-/ characters.
            
            # When ** matches zero segments, we effectively remove the ** and
            # the surrounding / appropriately.
            
            # Let me handle this by trying all possible numbers of segments to consume.
            
            # First, try ** matching zero segments
            # This means we just move to next token
            if match_tokens(tokens, ti + 1, path, pi):
                return True
            
            # Try ** matching one or more segments
            # We need to find positions where we can split into segments
            # Starting from pi, try consuming 1, 2, 3, ... segments
            
            # A segment starts at pi and ends before the next /
            # After consuming k segments, we should be at a position that
            # either is end of path or starts with /
            
            # Let's find all possible end positions after consuming segments
            # Starting from pi, find segment boundaries
            
            # Actually, let me think about this more carefully.
            # If we have pattern a/**/b and path a/x/y/b
            # tokens: literal(a), literal(/), **, literal(/), literal(b)
            # Wait, no. The / in the pattern are literal characters.
            
            # Hmm, this is getting complicated. Let me reconsider.
            
            # Actually, in the pattern, / are literal characters.
            # So a/**/b parses as: literal(a), literal(/), **, literal(/), literal(b)
            # And path a/x/y/b needs to match.
            
            # When ** matches zero segments, we want a//b to match a/x/y/b? No.
            # When ** matches zero segments, the pattern effectively becomes a/b
            # So we need to handle the / around ** specially.
            
            # Let me reconsider the tokenization. Maybe I should handle ** differently.
            
            # Actually, let me try a different approach. Let me handle ** by
            # trying all possible ways to split the path into segments and
            # matching segments against pattern segments.
            
            # But that's complex too. Let me try the recursive approach more carefully.
            
            # For ** matching zero segments: we skip ** and try to match.
            # But the / before and after ** in the pattern are separate tokens.
            # So if pattern is a/**/b, tokens are [a, /, **, /, b]
            # If ** matches zero segments, we need a//b to match... which won't work.
            
            # I think the issue is that ** should absorb the surrounding / when
            # it matches zero segments. Let me handle this in the matching logic.
            
            # When ** matches zero segments, we should skip the ** AND handle
            # the / appropriately. Specifically:
            # - If ** is preceded by / in pattern, skip that /
            # - If ** is followed by / in pattern, skip that /
            
            # This is getting messy. Let me try a cleaner approach.
            
            # Alternative: preprocess the pattern to handle ** properly.
            # Replace /**/ with / and /** at end with nothing, etc.
            # But that loses information.
            
            # Let me try yet another approach: convert pattern to regex.
            
            # Actually, let me just handle ** in the recursive matcher by
            # trying all possible segment splits.
            
            # For ** at position ti, try matching tokens[ti+1:] against
            # path starting at various positions.
            
            # The positions we try should be at segment boundaries.
            # A segment boundary is either at the start of path or after a /.
            
            # First, try ** matching zero segments:
            # This means we just try tokens[ti+1:] against path[pi:]
            # But we also need to handle the / that might be in the pattern.
            
            # Hmm, let me just try all possible end positions for ** and
            # see if the rest matches.
            
            # For ** matching, it can consume any number of characters including /
            # as long as it aligns with segment boundaries.
            
            # Let me try: for each possible end position ep >= pi,
            # check if path[pi:ep] consists of complete segments (i.e., 
            # path[pi:ep] is empty or ends with / or the next char after ep is / or ep is end)
            # Actually, ** matches whole segments, so path[pi:ep] should be
            # a sequence of complete segments.
            
            # A sequence of complete segments means:
            # - Empty (zero segments)
            # - Or: seg1/seg2/.../segN where each seg is non-empty and no / at start
            
            # So path[pi:ep] should either be empty, or
            # path[pi] != '/' and (ep == len(path) or path[ep] == '/')
            # and no consecutive // in path[pi:ep]
            
            # Let me simplify: try all ep from pi to len(path),
            # and for each, check if path[pi:ep] is a valid sequence of segments
            # and if tokens[ti+1:] matches path[ep:]
            
            for ep in range(pi, len(path) + 1):
                # Check if path[pi:ep] is a valid sequence of segments
                # Valid means: empty, or starts with non-/, ends with / or end,
                # and no // inside
                segment_str = path[pi:ep]
                if segment_str == '':
                    # Zero segments, valid
                    pass
                else:
                    # Must not start with /
                    if segment_str[0] == '/':
                        continue
                    # Must not contain //
                    if '//' in segment_str:
                        continue
                    # If ep < len(path), next char must be /
                    if ep < len(path) and path[ep] != '/':
                        continue
                
                if match_tokens(tokens, ti + 1, path, ep):
                    return True
            
            return False
        
        elif token_type == 'class':
            negated, char_set = token_data
            if pi < len(path):
                c = path[pi]
                if c == '/':
                    return False
                in_set = c in char_set
                if negated:
                    if not in_set:
                        return match_tokens(tokens, ti + 1, path, pi + 1)
                else:
                    if in_set:
                        return match_tokens(tokens, ti + 1, path, pi + 1)
            return False
        
        return False
    
    tokens = parse_pattern(pattern)
    return match_tokens(tokens, 0, path, 0)