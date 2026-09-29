def match(pattern, path):
    """
    Returns True if a glob pattern matches a whole path.
    
    Rules:
    - Paths are separated by '/'.
    - '?' matches exactly one character but never '/'.
    - '*' matches zero or more characters but never '/'.
    - '**' as a whole segment matches zero or more whole segments.
    - Square brackets are a character class matching one character:
      '[abc]', ranges like '[a-z]', and '[!...]' for negation.
      A class never matches '/'.
    - A backslash escapes the next character so it is treated literally.
    - The whole path must match, not a prefix.
    """
    
    def parse_pattern(pattern):
        """Parse pattern into a list of tokens.
        Each token is either:
        - ('literal', char) for a literal character
        - ('?',) for single char wildcard
        - ('*',) for multi-char wildcard (not crossing /)
        - ('**',) for multi-segment wildcard
        - ('class', chars_set, negated) for character class
        """
        tokens = []
        i = 0
        n = len(pattern)
        while i < n:
            c = pattern[i]
            if c == '\\':
                # Escape next character
                if i + 1 < n:
                    tokens.append(('literal', pattern[i + 1]))
                    i += 2
                else:
                    # Backslash at end, treat as literal backslash
                    tokens.append(('literal', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('?',))
                i += 1
            elif c == '*':
                # Check if it's **
                if i + 1 < n and pattern[i + 1] == '*':
                    # Check if this ** is a whole segment
                    # It's a whole segment if it's at start or preceded by /, and at end or followed by /
                    is_start = (i == 0 or pattern[i - 1] == '/')
                    is_end = (i + 2 >= n or pattern[i + 2] == '/')
                    if is_start and is_end:
                        tokens.append(('**',))
                        i += 2
                    else:
                        # Not a whole segment **, treat as two * or just *
                        # Actually, per spec, ** only has special meaning as a whole segment.
                        # If not a whole segment, we should treat it as two * tokens? 
                        # But the spec says "A path segment that is exactly '**'".
                        # So if it's not a whole segment, it's not special.
                        # Let's treat each * separately.
                        tokens.append(('*',))
                        i += 1
                else:
                    tokens.append(('*',))
                    i += 1
            elif c == '[':
                # Parse character class
                i += 1
                negated = False
                if i < n and pattern[i] == '!':
                    negated = True
                    i += 1
                chars = set()
                # Handle ] as first char in class
                if i < n and pattern[i] == ']':
                    chars.add(']')
                    i += 1
                while i < n and pattern[i] != ']':
                    if pattern[i] == '\\':
                        if i + 1 < n:
                            chars.add(pattern[i + 1])
                            i += 2
                        else:
                            chars.add('\\')
                            i += 1
                    elif i + 2 < n and pattern[i + 1] == '-' and pattern[i + 2] != ']':
                        # Range
                        start = pattern[i]
                        end = pattern[i + 2]
                        for code in range(ord(start), ord(end) + 1):
                            chars.add(chr(code))
                        i += 3
                    else:
                        chars.add(pattern[i])
                        i += 1
                if i < n and pattern[i] == ']':
                    i += 1
                tokens.append(('class', chars, negated))
            elif c == '/':
                tokens.append(('literal', '/'))
                i += 1
            else:
                tokens.append(('literal', c))
                i += 1
        return tokens
    
    def match_tokens(tokens, ti, path, pi):
        """
        Recursively match tokens[ti:] against path[pi:].
        Returns True if match succeeds.
        """
        # Base case: no more tokens
        if ti == len(tokens):
            return pi == len(path)
        
        token = tokens[ti]
        
        if token[0] == 'literal':
            if pi < len(path) and path[pi] == token[1]:
                return match_tokens(tokens, ti + 1, path, pi + 1)
            return False
        
        elif token[0] == '?':
            if pi < len(path) and path[pi] != '/':
                return match_tokens(tokens, ti + 1, path, pi + 1)
            return False
        
        elif token[0] == '*':
            # * matches zero or more characters but never '/'
            # Try matching zero chars first, then one, two, etc.
            # We need to find all positions where we can match the rest
            # Since * doesn't cross /, we stop at /
            for k in range(pi, len(path) + 1):
                if k > pi and path[k - 1] == '/':
                    break
                if match_tokens(tokens, ti + 1, path, k):
                    return True
            return False
        
        elif token[0] == '**':
            # ** matches zero or more whole segments
            # This means it can match:
            # - zero segments (nothing)
            # - one or more segments (each segment is a sequence of non-/ chars, separated by /)
            
            # Case 1: ** matches zero segments
            # Then we need to match tokens[ti+1:] against path[pi:]
            # But we need to handle the / that might be around **
            # Actually, ** as a segment means it replaces the entire segment including surrounding slashes appropriately.
            
            # Let's think about this differently. The tokens include literal '/' characters.
            # So if we have tokens like: literal 'a', literal '/', **, literal '/', literal 'b'
            # For path 'a/b': after matching 'a', we're at '/'. Then ** should match zero segments,
            # meaning we skip the ** and the following '/' should match the '/' in path.
            
            # Hmm, this is tricky. Let me reconsider the tokenization.
            
            # Actually, let me re-think. The ** token represents a whole segment wildcard.
            # When we encounter **, it can match:
            # - Nothing (zero segments), in which case we need to handle the surrounding slashes
            # - One or more segments
            
            # Let me try a different approach: handle ** by trying all possible splits.
            
            # ** matches zero or more whole segments. A "whole segment" is a sequence of chars between / boundaries.
            
            # Option 1: ** matches zero segments.
            # In this case, we effectively remove the ** token and try to match.
            # But we need to be careful about slashes.
            
            # Let me try: for ** matching zero segments, we just skip the ** token.
            # But what about the slashes around it? They are separate literal tokens.
            
            # Example: pattern 'a/**/b' tokenizes to: literal 'a', literal '/', **, literal '/', literal 'b'
            # Path 'a/b': 
            #   Match literal 'a' -> pi=1
            #   Match literal '/' -> pi=2
            #   Now at ** token, pi=2, path='a/b', remaining path is 'b'
            #   ** matches zero segments: skip **, try to match literal '/' at pi=2. path[2]='b' != '/'. Fail.
            #   ** matches one segment: the segment is 'b'. After matching, pi should be at end.
            #   But then we still have literal '/' and literal 'b' to match. Fail.
            
            # This approach isn't working well. Let me reconsider.
            
            # The issue is that ** as a segment wildcard needs to consume the segment AND handle the slashes.
            
            # Let me try a different tokenization strategy or matching strategy.
            
            # Alternative: When we see **, we know it's a whole segment. So it should match:
            # - Zero segments: this means the ** effectively disappears, and the surrounding slashes merge or one disappears.
            # - One or more segments: it consumes those segments.
            
            # Let me try: for **, try matching zero segments by skipping the ** and also potentially skipping an adjacent slash.
            
            # Actually, let me try a cleaner recursive approach where ** tries all possible numbers of segments to consume.
            
            # For ** at position ti, it can match 0, 1, 2, ... segments.
            # A segment is a maximal sequence of non-/ characters.
            
            # If ** matches 0 segments: we skip the ** token. But we need to handle the slash situation.
            # If ** matches k segments (k >= 1): we consume k segments from the path.
            
            # Let me try: 
            # Try 0 segments: match tokens[ti+1:] against path[pi:]
            # Try 1+ segments: find the next / after pi, that's one segment. Then try matching tokens[ti:] (still **) against the rest.
            
            # Wait, that's not quite right either because ** can match multiple segments.
            
            # Let me try: ** can match zero or more segments. So:
            # Option A: ** matches zero segments. Skip the ** token and try to match the rest.
            # Option B: ** matches at least one segment. Consume one segment (up to next / or end), then try to match ** again (same token) against the rest.
            
            # But we need to handle the slashes properly. Let me think about what "consume one segment" means.
            # If we're at position pi in path, and we want to consume one segment:
            # - Find the next / starting from pi. Let's say it's at position j.
            # - The segment is path[pi:j].
            # - After consuming, we're at position j (the / position) or len(path) if no /.
            
            # Hmm, but the slashes are part of the path structure. Let me try yet another approach.
            
            # Let me handle ** by trying all possible end positions for the segments it consumes.
            # ** matches zero or more whole segments. So it can match:
            # - Empty string (zero segments)
            # - A sequence of segments like "seg1/seg2/.../segk"
            
            # When ** matches zero segments, we just skip the ** token.
            # When ** matches one or more segments, we need to find where those segments end.
            
            # Let me try: for each possible position where ** could end (after consuming 0, 1, 2, ... segments),
            # try to match the remaining tokens.
            
            # To find positions after consuming k segments:
            # Start at pi. 
            # After 0 segments: position pi.
            # After 1 segment: find next / from pi. If found at j, position is j (but we need to handle the /).
            # This is getting complicated with the slashes.
            
            # Let me try a simpler recursive approach:
            # ** can match zero segments: try match_tokens(tokens, ti+1, path, pi)
            # ** can match one or more segments: 
            #   Find the end of the current segment (next / or end of path).
            #   Let's say the segment ends at position j (j is the position of / or len(path)).
            #   After consuming this segment, we're at position j.
            #   But we also need to consume the / if there is one, because the next segment starts after /.
            #   Actually, let's say we consume the segment and the following / if present.
            #   Then try match_tokens(tokens, ti, path, new_pi) where new_pi is after the segment and optional /.
            
            # Hmm, let me just try all possible split points.
            
            # For ** matching zero or more segments:
            # Try: ** matches nothing. Continue with ti+1 at pi.
            # Try: ** matches one or more segments. 
            #   Find all positions where a segment boundary occurs.
            
            # Let me try this: 
            # Option 1: ** matches zero segments.
            if match_tokens(tokens, ti + 1, path, pi):
                return True
            
            # Option 2: ** matches at least one segment.
            # Find the end of the current segment starting at pi.
            # A segment is chars until next / or end.
            j = pi
            while j < len(path) and path[j] != '/':
                j += 1
            # Now j is at the next / or end of path.
            # The segment is path[pi:j].
            # After consuming this segment, if j < len(path), we're at the /. 
            # We need to also consume the / to move to the next segment.
            # So new position is j + 1 if j < len(path), else j.
            # But wait, if we consume the segment and the /, we're at the start of the next segment.
            # Then we try ** again (same token) from there.
            
            if j < len(path):
                # There's a / at position j. Consume segment and the /.
                new_pi = j + 1
                if match_tokens(tokens, ti, path, new_pi):
                    return True
            else:
                # j == len(path), we consumed the last segment.
                # After consuming, we're at end of path.
                # Try matching ** again (which can match zero segments) at end.
                if match_tokens(tokens, ti, path, j):
                    return True
            
            return False
        
        elif token[0] == 'class':
            chars, negated = token[1], token[2]
            if pi < len(path) and path[pi] != '/':
                c = path[pi]
                if negated:
                    if c not in chars:
                        return match_tokens(tokens, ti + 1, path, pi + 1)
                else:
                    if c in chars:
                        return match_tokens(tokens, ti + 1, path, pi + 1)
            return False
        
        return False
    
    tokens = parse_pattern(pattern)
    return match_tokens(tokens, 0, path, 0)