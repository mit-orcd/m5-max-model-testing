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
    # Tokenize the pattern into segments
    # We need to handle '**' as a special segment
    
    # First, let's parse the pattern into a list of segments
    # Each segment is either '**' or a list of pattern characters
    
    def parse_pattern(pattern):
        segments = []
        i = 0
        n = len(pattern)
        while i < n:
            # Check if this is a '**' segment
            if pattern[i] == '/' or i == 0:
                # Start of a new segment
                if i == 0:
                    # Check if the whole pattern is '**'
                    if pattern == '**':
                        return ['**']
                    # Find the end of this segment
                    j = i
                    while j < n and pattern[j] != '/':
                        j += 1
                    seg = pattern[i:j]
                    if seg == '**':
                        segments.append('**')
                    else:
                        segments.append(seg)
                    i = j
                else:
                    # pattern[i] == '/'
                    # Check if the next segment is '**'
                    j = i + 1
                    while j < n and pattern[j] != '/':
                        j += 1
                    seg = pattern[i+1:j]
                    if seg == '**':
                        segments.append('**')
                    else:
                        segments.append(seg)
                    i = j
            else:
                # This shouldn't happen if we handle it correctly
                # But let's handle the case where we're in the middle
                j = i
                while j < n and pattern[j] != '/':
                    j += 1
                seg = pattern[i:j]
                if seg == '**':
                    segments.append('**')
                else:
                    segments.append(seg)
                i = j
        return segments
    
    # Actually, let me redo the parsing more carefully
    def parse_pattern(pattern):
        segments = []
        i = 0
        n = len(pattern)
        while i < n:
            # Skip leading '/'
            if pattern[i] == '/':
                i += 1
                continue
            # Find the end of this segment
            j = i
            while j < n and pattern[j] != '/':
                j += 1
            seg = pattern[i:j]
            if seg == '**':
                segments.append('**')
            else:
                segments.append(seg)
            i = j
        return segments
    
    pattern_segments = parse_pattern(pattern)
    path_segments = path.split('/')
    
    # Now we need to match pattern_segments against path_segments
    # Use dynamic programming or backtracking
    
    # Let's use a recursive approach with memoization
    from functools import lru_cache
    
    @lru_cache(maxsize=None)
    def match_segment(pattern_seg, path_seg):
        """
        Match a single pattern segment against a single path segment.
        pattern_seg is a string (not '**'), path_seg is a string.
        Returns True if the pattern segment matches the path segment.
        """
        i = 0
        j = 0
        n = len(pattern_seg)
        m = len(path_seg)
        
        # Use DP for matching within a segment
        # dp[i][j] = True if pattern_seg[i:] matches path_seg[j:]
        dp = [[False] * (m + 1) for _ in range(n + 1)]
        dp[n][m] = True
        
        for i in range(n - 1, -1, -1):
            for j in range(m - 1, -1, -1):
                if pattern_seg[i] == '\\':
                    # Escape: next character is literal
                    if i + 1 < n:
                        literal = pattern_seg[i + 1]
                        if path_seg[j] == literal:
                            dp[i][j] = dp[i + 2][j + 1]
                        else:
                            dp[i][j] = False
                    else:
                        dp[i][j] = False
                elif pattern_seg[i] == '?':
                    # Matches exactly one character, but never '/'
                    # Since we're within a segment, path_seg[j] is never '/'
                    dp[i][j] = dp[i + 1][j + 1]
                elif pattern_seg[i] == '*':
                    # Matches zero or more characters, but never '/'
                    # Within a segment, all characters are non-'/'
                    # dp[i][j] = dp[i+1][j] (zero chars) or dp[i][j+1] (one more char)
                    dp[i][j] = dp[i + 1][j] or dp[i][j + 1]
                elif pattern_seg[i] == '[':
                    # Character class
                    # Find the closing ']'
                    k = i + 1
                    negate = False
                    if k < n and pattern_seg[k] == '!':
                        negate = True
                        k += 1
                    # Find the closing bracket
                    close = k
                    while close < n and pattern_seg[close] != ']':
                        close += 1
                    if close >= n:
                        # No closing bracket, treat '[' as literal
                        if path_seg[j] == '[':
                            dp[i][j] = dp[i + 1][j + 1]
                        else:
                            dp[i][j] = False
                    else:
                        # Parse the character class
                        # The class is pattern_seg[k:close]
                        # Check if path_seg[j] is in the class
                        char = path_seg[j]
                        in_class = False
                        p = k
                        while p < close:
                            if p + 2 < close and pattern_seg[p + 1] == '-':
                                # Range
                                start = pattern_seg[p]
                                end = pattern_seg[p + 2]
                                if start <= char <= end:
                                    in_class = True
                                    break
                                p += 3
                            else:
                                if char == pattern_seg[p]:
                                    in_class = True
                                    break
                                p += 1
                        if negate:
                            in_class = not in_class
                        if in_class:
                            dp[i][j] = dp[close + 1][j + 1]
                        else:
                            dp[i][j] = False
                else:
                    # Literal character
                    if path_seg[j] == pattern_seg[i]:
                        dp[i][j] = dp[i + 1][j + 1]
                    else:
                        dp[i][j] = False
        
        return dp[0][0]
    
    def match_segments(pi, si):
        """
        Match pattern_segments[pi:] against path_segments[si:].
        pi: index in pattern_segments
        si: index in path_segments
        """
        if pi == len(pattern_segments):
            return si == len(path_segments)
        if si == len(path_segments):
            # No more path segments, but still pattern segments
            # Only valid if all remaining pattern segments are '**'
            for k in range(pi, len(pattern_segments)):
                if pattern_segments[k] != '**':
                    return False
            return True
        
        if pattern_segments[pi] == '**':
            # '**' matches zero or more whole segments
            # Option 1: match zero segments (skip '**')
            # Option 2: match one or more segments
            # Try skipping '**'
            if match_segments(pi + 1, si):
                return True
            # Try consuming one path segment and staying at '**'
            if match_segments(pi, si + 1):
                return True
            return False
        else:
            # Match this pattern segment against this path segment
            if match_segment(pattern_segments[pi], path_segments[si]):
                return match_segments(pi + 1, si + 1)
            return False
    
    return match_segments(0, 0)