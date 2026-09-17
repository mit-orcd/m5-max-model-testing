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
        """Parse pattern into segments. Each segment is a list of tokens."""
        segments = []
        current_segment = []
        i = 0
        n = len(pattern)
        
        while i < n:
            if pattern[i] == '/':
                segments.append(current_segment)
                current_segment = []
                i += 1
            else:
                if pattern[i] == '\\' and i + 1 < n:
                    # Escaped character
                    current_segment.append(('literal', pattern[i + 1]))
                    i += 2
                elif pattern[i] == '?':
                    current_segment.append(('question', None))
                    i += 1
                elif pattern[i] == '*':
                    current_segment.append(('star', None))
                    i += 1
                elif pattern[i] == '[':
                    # Parse character class
                    j = i + 1
                    negate = False
                    if j < n and pattern[j] == '!':
                        negate = True
                        j += 1
                    chars = []
                    while j < n and pattern[j] != ']':
                        if pattern[j] == '\\' and j + 1 < n:
                            chars.append(pattern[j + 1])
                            j += 2
                        elif j + 2 < n and pattern[j + 1] == '-':
                            # Range
                            start = pattern[j]
                            end = pattern[j + 2]
                            for c in range(ord(start), ord(end) + 1):
                                chars.append(chr(c))
                            j += 3
                        else:
                            chars.append(pattern[j])
                            j += 1
                    if j < n and pattern[j] == ']':
                        j += 1
                    current_segment.append(('class', (negate, chars)))
                    i = j
                else:
                    current_segment.append(('literal', pattern[i]))
                    i += 1
        
        segments.append(current_segment)
        return segments
    
    def parse_path(path):
        """Parse path into segments."""
        if path == '':
            return []
        return path.split('/')
    
    pattern_segments = parse_pattern(pattern)
    path_segments = parse_path(path)
    
    # Now we need to match pattern_segments against path_segments
    # Use dynamic programming or recursion with memoization
    
    # dp[i][j] = True if pattern_segments[i:] matches path_segments[j:]
    m = len(pattern_segments)
    n = len(path_segments)
    
    # Use memoization
    memo = {}
    
    def is_star_segment(seg):
        """Check if a segment is exactly '**' (a single star token)."""
        return len(seg) == 1 and seg[0][0] == 'star'
    
    def match_segment(seg, path_seg):
        """Check if a single pattern segment matches a single path segment."""
        if len(seg) == 0:
            return len(path_seg) == 0
        if len(path_seg) == 0:
            # Pattern segment is non-empty but path segment is empty
            # Only matches if all tokens can match zero characters
            # Star can match zero, but question, literal, class cannot
            for token in seg:
                if token[0] != 'star':
                    return False
            return True
        
        # Match character by character
        # Use DP within the segment
        # dp[i][j] = True if seg[i:] matches path_seg[j:]
        slen = len(seg)
        plen = len(path_seg)
        dp = [[False] * (plen + 1) for _ in range(slen + 1)]
        dp[slen][plen] = True
        
        for i in range(slen - 1, -1, -1):
            for j in range(plen - 1, -1, -1):
                token = seg[i]
                if token[0] == 'star':
                    # Star matches zero or more characters
                    # dp[i][j] = dp[i+1][j] (zero chars) or dp[i][j+1] (one more char)
                    dp[i][j] = dp[i + 1][j] or dp[i][j + 1]
                elif token[0] == 'question':
                    # Question matches exactly one character, not '/'
                    # But we're within a segment, so no '/' in path_seg
                    if j < plen:
                        dp[i][j] = dp[i + 1][j + 1]
                elif token[0] == 'literal':
                    if j < plen and path_seg[j] == token[1]:
                        dp[i][j] = dp[i + 1][j + 1]
                elif token[0] == 'class':
                    negate, chars = token[1]
                    if j < plen:
                        c = path_seg[j]
                        if negate:
                            if c not in chars:
                                dp[i][j] = dp[i + 1][j + 1]
                        else:
                            if c in chars:
                                dp[i][j] = dp[i + 1][j + 1]
        
        return dp[0][0]
    
    def dp(i, j):
        """Returns True if pattern_segments[i:] matches path_segments[j:]."""
        if (i, j) in memo:
            return memo[(i, j)]
        
        # Base cases
        if i == m and j == n:
            memo[(i, j)] = True
            return True
        if i == m:
            # No more pattern segments, but path segments remain
            memo[(i, j)] = False
            return False
        if j == n:
            # No more path segments, but pattern segments remain
            # All remaining pattern segments must be '**' (which can match zero segments)
            for k in range(i, m):
                if not is_star_segment(pattern_segments[k]):
                    memo[(i, j)] = False
                    return False
            memo[(i, j)] = True
            return True
        
        seg = pattern_segments[i]
        
        if is_star_segment(seg):
            # '**' matches zero or more whole segments
            # Option 1: match zero segments (skip the '**')
            # Option 2: match one or more segments (consume one path segment and stay at '**')
            result = dp(i + 1, j) or dp(i, j + 1)
            memo[(i, j)] = result
            return result
        else:
            # Regular segment: must match exactly one path segment
            if match_segment(seg, path_segments[j]):
                memo[(i, j)] = dp(i + 1, j + 1)
            else:
                memo[(i, j)] = False
            return memo[(i, j)]
    
    return dp(0, 0)