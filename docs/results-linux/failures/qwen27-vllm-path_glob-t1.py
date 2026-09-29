def match(pattern, path):
    """
    Returns True if a glob pattern matches a whole path.
    Paths are separated by '/'.
    '?' matches exactly one character but never '/'.
    '*' matches zero or more characters but never '/'.
    A path segment that is exactly '**' matches zero or more whole segments.
    Square brackets are a character class matching one character.
    A backslash escapes the next character so it is treated literally.
    The whole path must match, not a prefix.
    """
    
    def parse_pattern(pattern):
        """Parse pattern into a list of tokens."""
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
                tokens.append(('question', None))
                i += 1
            elif c == '*':
                # Check if this is part of '**'
                if i + 1 < n and pattern[i + 1] == '*':
                    # Check if it's a standalone '**' segment
                    # It should be preceded by start or '/' and followed by end or '/'
                    is_start = (i == 0 or pattern[i - 1] == '/')
                    is_end = (i + 2 >= n or pattern[i + 2] == '/')
                    if is_start and is_end:
                        tokens.append(('double_star', None))
                        i += 2
                    else:
                        # Not a standalone **, treat as regular *
                        tokens.append(('star', None))
                        i += 1
                else:
                    tokens.append(('star', None))
                    i += 1
            elif c == '[':
                # Parse character class
                j = i + 1
                negate = False
                if j < n and pattern[j] == '!':
                    negate = True
                    j += 1
                chars = []
                if j < n and pattern[j] == ']':
                    # ] is literal
                    chars.append(']')
                    j += 1
                while j < n and pattern[j] != ']':
                    if pattern[j] == '\\' and j + 1 < n:
                        chars.append(pattern[j + 1])
                        j += 2
                    elif j + 2 < n and pattern[j + 1] == '-' and pattern[j + 2] != ']':
                        # Range
                        start_c = pattern[j]
                        end_c = pattern[j + 2]
                        for k in range(ord(start_c), ord(end_c) + 1):
                            chars.append(chr(k))
                        j += 3
                    else:
                        chars.append(pattern[j])
                        j += 1
                if j < n and pattern[j] == ']':
                    j += 1
                tokens.append(('class', (negate, chars)))
                i = j
            elif c == '/':
                tokens.append(('slash', None))
                i += 1
            else:
                tokens.append(('literal', c))
                i += 1
        return tokens
    
    def match_char(token, ch):
        """Check if a single character token matches a character."""
        kind, val = token
        if kind == 'literal':
            return ch == val
        elif kind == 'question':
            return ch != '/'
        elif kind == 'class':
            negate, chars = val
            if ch == '/':
                return False
            if negate:
                return ch not in chars
            else:
                return ch in chars
        return False
    
    def is_char_token(token):
        """Check if token matches exactly one character."""
        kind, _ = token
        return kind in ('literal', 'question', 'class')
    
    tokens = parse_pattern(pattern)
    
    # We'll use memoized recursion
    from functools import lru_cache
    
    @lru_cache(maxsize=None)
    def dp(pi, si):
        """
        pi: index in tokens
        si: index in path
        Returns True if tokens[pi:] matches path[si:]
        """
        if pi == len(tokens):
            return si == len(path)
        if si == len(path):
            # No more path, check if remaining tokens can match empty
            # Only possible if all remaining tokens are stars that can match zero
            for i in range(pi, len(tokens)):
                kind, _ = tokens[i]
                if kind in ('literal', 'question', 'class', 'slash'):
                    return False
                # star and double_star can match zero
            return True
        
        token = tokens[pi]
        kind, val = token
        
        if kind == 'slash':
            # Must match a '/' in path
            if path[si] == '/':
                return dp(pi + 1, si + 1)
            return False
        
        elif kind == 'star':
            # * matches zero or more non-'/' characters
            # Option 1: match zero characters
            if dp(pi + 1, si):
                return True
            # Option 2: match one or more characters (not '/')
            if path[si] != '/':
                return dp(pi, si + 1)
            return False
        
        elif kind == 'double_star':
            # ** matches zero or more whole segments
            # Option 1: match zero segments
            if dp(pi + 1, si):
                return True
            # Option 2: match one or more segments
            # We need to find a position in path that aligns with the next token
            # The next token after ** should be a '/' (unless it's the last token)
            if pi + 1 < len(tokens):
                next_kind, _ = tokens[pi + 1]
                if next_kind == 'slash':
                    # Find all positions where path has '/' and try matching from there
                    # We can try each '/' position in path[si:]
                    for j in range(si, len(path)):
                        if path[j] == '/':
                            # Try matching from j (the slash in path)
                            # tokens[pi+1] is a slash, so it should match path[j]
                            if dp(pi + 1, j):
                                return True
                    # Also try matching the rest if no more slashes needed
                    # Actually, we should also consider that ** can consume up to the end
                    # Let's also try if the remaining tokens can match from some position
                    # The above loop handles slashes. We also need to handle the case
                    # where ** consumes everything and the rest matches empty
                    # That's covered by dp(pi+1, si) for zero, and for non-zero we need
                    # to find where the next part starts.
                    # Let me reconsider: after **, the next token is '/'. So we need
                    # path to have a '/' at some position j >= si, and then tokens[pi+1:]
                    # matches path[j:].
                    # But what if there's no more '/' in path? Then ** can't match 
                    # non-zero segments followed by a '/'.
                    # Actually, we should also consider that ** might be at the end
                    # or followed by something else. Let me re-examine.
                    # If next is not slash, then ** should match up to a segment boundary
                    # but the next token expects something else. This is tricky.
                    # Let me just try all possible positions where a segment could end.
                    # Actually, the standard approach: ** matches any number of segments.
                    # So we can try: for each position j in [si, len(path)], if path[j:]
                    # could be the start of the remaining pattern, try it. But we need
                    # to ensure segment boundaries.
                    # Simpler: try each '/' position and also the end.
                    # Let me redo this more carefully.
                    pass
                else:
                    # Next token is not a slash. This shouldn't happen in well-formed
                    # patterns, but let's handle it: ** should match up to a segment
                    # boundary, so the next token should start at a segment start.
                    # Try each position where a new segment starts (after a '/' or start)
                    for j in range(si, len(path) + 1):
                        # j is a valid segment start if j == len(path) or path[j-1] == '/' (if j > si)
                        # Actually, segment starts are at positions 0 and after each '/'
                        # Let's just try all j and see if dp(pi+1, j) works
                        if dp(pi + 1, j):
                            return True
                    return False
            else:
                # ** is the last token, matches everything
                return True
        
        elif kind in ('literal', 'question', 'class'):
            # Matches exactly one character
            if match_char(token, path[si]):
                return dp(pi + 1, si + 1)
            return False
        
        return False
    
    result = dp(0, 0)
    dp.cache_clear()
    return result