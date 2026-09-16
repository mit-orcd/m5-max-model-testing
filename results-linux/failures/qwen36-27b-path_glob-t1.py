def match(pattern, path):
    """
    Returns True if a glob pattern matches a whole path.
    """
    # Split pattern and path by '/'
    pattern_parts = pattern.split('/')
    path_parts = path.split('/')

    def match_segment(pattern_seg, path_seg):
        """
        Match a single path segment against a single pattern segment.
        Handles ?, *, [], and escapes.
        """
        pi = 0  # pattern index
        si = 0  # segment index
        plen = len(pattern_seg)
        slen = len(path_seg)

        # Use dynamic programming or recursive with memoization for * handling
        # Since segments don't contain '/', we can use a simple DP approach
        
        # dp[i][j] = True if pattern_seg[:i] matches path_seg[:j]
        dp = [[False] * (slen + 1) for _ in range(plen + 1)]
        dp[0][0] = True

        for i in range(1, plen + 1):
            # Handle leading *s
            if pattern_seg[i-1] == '*':
                dp[i][0] = dp[i-1][0]
            else:
                break

        for i in range(1, plen + 1):
            for j in range(1, slen + 1):
                p_char = pattern_seg[i-1]
                s_char = path_seg[j-1]

                if p_char == '?':
                    # Matches any single character except '/' (but '/' is already excluded by segment split)
                    dp[i][j] = dp[i-1][j-1]
                elif p_char == '*':
                    # Matches zero or more characters
                    dp[i][j] = dp[i-1][j] or dp[i][j-1]
                elif p_char == '[':
                    # Find the closing bracket
                    k = i
                    while k < plen and pattern_seg[k] != ']':
                        k += 1
                    if k == plen:
                        # No closing bracket, treat '[' as literal
                        dp[i][j] = dp[i-1][j-1] and (pattern_seg[i-1] == s_char)
                    else:
                        # Extract the class content
                        class_content = pattern_seg[i:k]
                        # Check if negated
                        negated = False
                        start = 0
                        if class_content and class_content[0] == '!':
                            negated = True
                            start = 1
                        
                        matched = False
                        c = start
                        while c < len(class_content):
                            # Handle escape in class
                            if class_content[c] == '\\':
                                c += 1
                                if c < len(class_content):
                                    literal = class_content[c]
                                    if s_char == literal:
                                        matched = True
                                    c += 1
                                continue
                            
                            # Handle range
                            if c + 2 < len(class_content) and class_content[c+1] == '-' and class_content[c+2] != ']':
                                lower = class_content[c]
                                upper = class_content[c+2]
                                if lower <= s_char <= upper:
                                    matched = True
                                c += 3
                            else:
                                # Single character
                                if s_char == class_content[c]:
                                    matched = True
                                c += 1
                        
                        if negated:
                            matched = not matched
                        
                        # The class matches one character, so we advance both indices by the class length
                        dp[i][j] = dp[i-1][j-1] and matched
                        # Skip the rest of the class in the outer loop by marking subsequent positions
                        # Actually, we need to skip the pattern indices for the class
                        # Let's handle this differently: consume the whole class at once
                        # But our DP is character-by-character. Let's adjust.
                        # If we hit '[', we need to consume the entire [...] as one unit.
                        # This complicates the DP. Let's use a different approach for segments with classes.
                        pass
                elif p_char == '\\':
                    # Escape: next character is literal
                    if i < plen:
                        next_char = pattern_seg[i]
                        dp[i+1][j] = dp[i-1][j-1] and (next_char == s_char)
                        # We need to skip the next character in the pattern loop
                        # This is tricky in a simple DP. Let's preprocess the pattern.
                        pass
                else:
                    dp[i][j] = dp[i-1][j-1] and (p_char == s_char)

        # The above DP doesn't correctly handle [...] and escapes in a single pass.
        # Let's rewrite match_segment to preprocess the pattern into a list of matchers.
        return False

    # Better approach: preprocess pattern segments into a list of matchers
    def preprocess_segment(seg):
        """
        Convert a pattern segment into a list of matchers.
        Each matcher is a function that takes a character and returns True/False,
        or a special marker for '*' which matches zero or more.
        """
        matchers = []
        i = 0
        n = len(seg)
        while i < n:
            c = seg[i]
            if c == '*':
                matchers.append('*')
                i += 1
            elif c == '?':
                matchers.append('?')
                i += 1
            elif c == '[':
                # Find closing bracket
                j = i + 1
                while j < n and seg[j] != ']':
                    if seg[j] == '\\':
                        j += 1  # skip escaped char
                    j += 1
                if j >= n:
                    # No closing bracket, treat '[' as literal
                    matchers.append(seg[i])
                    i += 1
                else:
                    # Extract class
                    class_str = seg[i+1:j]
                    negated = False
                    start = 0
                    if class_str and class_str[0] == '!':
                        negated = True
                        start = 1
                    
                    # Parse class into a set of characters or ranges
                    # For simplicity, we'll create a lambda that checks membership
                    def make_class_checker(class_str, negated):
                        def checker(ch):
                            s = class_str
                            idx = 0
                            if negated and s[0] == '!':
                                s = s[1:]
                            
                            matched = False
                            while idx < len(s):
                                if s[idx] == '\\':
                                    idx += 1
                                    if idx < len(s):
                                        if ch == s[idx]:
                                            matched = True
                                        idx += 1
                                    continue
                                
                                # Check for range
                                if idx + 2 < len(s) and s[idx+1] == '-' and s[idx+2] != ']':
                                    lower = s[idx]
                                    upper = s[idx+2]
                                    if lower <= ch <= upper:
                                        matched = True
                                    idx += 3
                                else:
                                    if ch == s[idx]:
                                        matched = True
                                    idx += 1
                            
                            return not matched if negated else matched
                        return checker
                    
                    matchers.append(make_class_checker(class_str, negated))
                    i = j + 1
            elif c == '\\':
                # Escape next character
                if i + 1 < n:
                    matchers.append(seg[i+1])
                    i += 2
                else:
                    matchers.append('\\')
                    i += 1
            else:
                matchers.append(c)
                i += 1
        return matchers

    def match_seg_with_matchers(matchers, path_seg):
        """
        Match a path segment against a list of matchers.
        """
        si = 0
        pi = 0
        mlen = len(matchers)
        slen = len(path_seg)
        
        # DP: dp[i][j] = True if matchers[:i] matches path_seg[:j]
        dp = [[False] * (slen + 1) for _ in range(mlen + 1)]
        dp[0][0] = True
        
        # Handle leading '*' matchers
        for i in range(1, mlen + 1):
            if matchers[i-1] == '*':
                dp[i][0] = dp[i-1][0]
            else:
                break
        
        for i in range(1, mlen + 1):
            for j in range(1, slen + 1):
                matcher = matchers[i-1]
                ch = path_seg[j-1]
                
                if matcher == '*':
                    dp[i][j] = dp[i-1][j] or dp[i][j-1]
                elif matcher == '?':
                    dp[i][j] = dp[i-1][j-1]
                elif callable(matcher):
                    dp[i][j] = dp[i-1][j-1] and matcher(ch)
                else:
                    dp[i][j] = dp[i-1][j-1] and (matcher == ch)
        
        return dp[mlen][slen]

    # Now match the whole path
    # We need to handle '**' which spans multiple segments
    # Convert pattern_parts into a list where '**' is a special token
    # Then use DP on the segments
    
    # Preprocess pattern parts
    pattern_tokens = []
    for part in pattern_parts:
        if part == '**':
            pattern_tokens.append('**')
        else:
            pattern_tokens.append(preprocess_segment(part))
    
    # DP on segments: dp[i][j] = True if pattern_tokens[:i] matches path_parts[:j]
    plen = len(pattern_tokens)
    slen = len(path_parts)
    dp = [[False] * (slen + 1) for _ in range(plen + 1)]
    dp[0][0] = True
    
    for i in range(1, plen + 1):
        token = pattern_tokens[i-1]
        if token == '**':
            # '**' can match zero or more segments
            dp[i][0] = dp[i-1][0]  # match zero segments
            for j in range(1, slen + 1):
                dp[i][j] = dp[i-1][j] or dp[i][j-1]
        else:
            # Regular segment matcher
            for j in range(1, slen + 1):
                # Match token against path_parts[j-1]
                if match_seg_with_matchers(token, path_parts[j-1]):
                    dp[i][j] = dp[i-1][j-1]
    
    return dp[plen][slen]