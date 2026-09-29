def match(pattern, path):
    """
    Returns True if a glob pattern matches a whole path.
    """
    # We will use a recursive approach with memoization or iterative DP.
    # Given the constraints and potential for deep recursion with **, 
    # an iterative DP or careful recursion is needed.
    # Let's parse the pattern into tokens first to handle ** and character classes efficiently.
    
    # Token types:
    # LITERAL: a single character (escaped or normal)
    # STAR: matches zero or more non-slash characters
    # DSTAR: matches zero or more segments (including empty)
    # CHAR_CLASS: matches one character from a set
    
    # However, a simpler approach is to process the pattern and path character by character
    # using dynamic programming or recursion with memoization.
    
    # Let's define a recursive function with memoization.
    # state: (pattern_index, path_index)
    
    memo = {}
    
    def dp(pi, si):
        # pi: index in pattern
        # si: index in path
        
        if (pi, si) in memo:
            return memo[(pi, si)]
        
        # Base cases
        if pi == len(pattern) and si == len(path):
            return True
        if pi == len(pattern):
            return False
        if si == len(path):
            # If pattern remains, it must be able to match empty string
            # Only ** can match empty string (zero segments)
            # Check if remaining pattern is all **
            for i in range(pi, len(pattern)):
                if pattern[i] != '*':
                    return False
                # If we see **, it can match zero segments
                # But if we see *, it cannot match zero segments if there's a slash later?
                # Actually, * never matches '/', so if path is empty, * cannot match anything.
                # So if path is empty, only ** (which matches zero segments) can succeed.
                # But wait, ** is two stars. Let's handle ** specially.
                # If we have a single *, it cannot match empty path because it needs to match chars but not '/', 
                # and there are no chars. So single * fails on empty path.
                # ** matches zero or more segments. Zero segments means it matches empty.
                # So if remaining pattern is "**", it can match.
                # If remaining pattern is "***", it's like "**" + "*", which fails.
                # Let's just check if the rest of the pattern can match empty path.
                pass
            # Let's handle this in the logic below.
            # If path is empty, we need to see if the rest of the pattern matches empty.
            # This is tricky. Let's rely on the general logic.
            pass
        
        # Check for **
        if pi < len(pattern) - 1 and pattern[pi] == '*' and pattern[pi+1] == '*':
            # It's a **
            # ** matches zero or more whole segments.
            # Zero segments: skip the ** and match the rest of the pattern with the rest of the path.
            # One or more segments: skip the ** and one segment from the path, then try again.
            # But ** can also match across slashes.
            # Strategy:
            # 1. Match zero segments: dp(pi + 2, si)
            # 2. Match one or more segments: if path[si] is not '/', we can consume it and try dp(pi + 2, si + 1)
            #    But wait, ** matches whole segments. So if we are at the start of a segment, we can skip the whole segment.
            #    Actually, the standard interpretation is:
            #    ** matches any number of directories (including zero).
            #    So 'a/**/b' matches 'a/b' (zero dirs) and 'a/x/b' (one dir) and 'a/x/y/b' (two dirs).
            #    Implementation:
            #    Option 1: Skip ** entirely (match zero segments).
            #    Option 2: If current path char is not '/', consume it and try to match ** again? No.
            #    Better: ** matches zero or more segments.
            #    So we can either:
            #    a) Skip the ** and match the rest of the pattern with the current path.
            #    b) If the current path character is not '/', consume it and try to match the ** again? 
            #       No, because ** matches whole segments.
            #    Let's think differently.
            #    When we encounter **, we can:
            #    1. Treat it as matching zero segments: dp(pi + 2, si)
            #    2. If si < len(path) and path[si] != '/', we can consume path[si] and try dp(pi, si + 1)? 
            #       No, because ** is a single token.
            #    Standard approach for ** in glob:
            #    ** matches any sequence of characters including '/'.
            #    But the problem says: "A path segment that is exactly '**' matches zero or more whole segments"
            #    This implies ** is a segment itself.
            #    So 'a/**/b' means: segment 'a', then ** (which matches 0 or more segments), then segment 'b'.
            #    So if we are at **, we can:
            #    - Match 0 segments: skip **, match rest of pattern with rest of path.
            #    - Match 1 or more segments: skip **, skip one segment from path, then try again.
            #    How to skip one segment? Find the next '/' or end of path.
            
            # Let's implement this logic.
            # Option 1: Match zero segments
            if dp(pi + 2, si):
                memo[(pi, si)] = True
                return True
            
            # Option 2: Match one or more segments
            # We need to skip one or more segments from the path.
            # We can try skipping 1 segment, 2 segments, etc.
            # But this can be inefficient.
            # Alternative: ** can match any number of segments.
            # So we can try: dp(pi + 2, si + k) for k = 1 to len(path) - si, where we skip whole segments.
            # But we can optimize: if we skip one segment, we can then try dp(pi + 2, new_si) again.
            # Actually, we can just try:
            # For each possible end of the first segment to skip:
            #   if dp(pi + 2, end_of_segment): return True
            
            # Find all segment boundaries
            j = si
            while j < len(path):
                if path[j] == '/':
                    # Skip the segment ending at j
                    if dp(pi + 2, j + 1):
                        memo[(pi, si)] = True
                        return True
                    j += 1
                else:
                    j += 1
            # Also consider the case where we skip until the end of the path
            if dp(pi + 2, len(path)):
                memo[(pi, si)] = True
                return True
                
            memo[(pi, si)] = False
            return False
        
        # Check for character class
        if pi < len(pattern) and pattern[pi] == '[':
            # Find the closing bracket
            j = pi + 1
            if j >= len(pattern):
                memo[(pi, si)] = False
                return False
            # Check for negation
            negate = False
            if j < len(pattern) and pattern[j] == '!':
                negate = True
                j += 1
            if j >= len(pattern) or pattern[j] != ']':
                # Invalid class, treat '[' as literal? Or fail?
                # Let's assume valid patterns.
                memo[(pi, si)] = False
                return False
            
            # Parse the class
            # We need to match exactly one character from the class
            if si >= len(path):
                memo[(pi, si)] = False
                return False
            
            char = path[si]
            if char == '/':
                memo[(pi, si)] = False
                return False
            
            # Parse the class content
            # We need to handle ranges like a-z
            # Let's collect all matching characters
            # But we can just check if char is in the class
            
            # Find the end of the class
            end_class = j
            while end_class < len(pattern) and pattern[end_class] != ']':
                end_class += 1
            if end_class >= len(pattern):
                memo[(pi, si)] = False
                return False
            
            # Now parse from j to end_class - 1
            # We need to check if char is in the class
            in_class = False
            k = j
            while k < end_class:
                if k + 2 < end_class and pattern[k+1] == '-' and pattern[k+2] != ']':
                    # Range
                    start_char = pattern[k]
                    end_char = pattern[k+2]
                    if start_char <= char <= end_char:
                        in_class = True
                        k += 3
                        continue
                    else:
                        k += 3
                        continue
                else:
                    if pattern[k] == char:
                        in_class = True
                    k += 1
            
            if negate:
                in_class = not in_class
            
            if in_class:
                res = dp(pi + (end_class - pi + 1), si + 1)
                memo[(pi, si)] = res
                return res
            else:
                memo[(pi, si)] = False
                return False
        
        # Check for literal or *
        if pi < len(pattern):
            p_char = pattern[pi]
            if p_char == '\\':
                # Escaped character
                if pi + 1 >= len(pattern):
                    memo[(pi, si)] = False
                    return False
                p_char = pattern[pi + 1]
                if si < len(path) and path[si] == p_char:
                    res = dp(pi + 2, si + 1)
                    memo[(pi, si)] = res
                    return res
                else:
                    memo[(pi, si)] = False
                    return False
            elif p_char == '*':
                # * matches zero or more characters but never '/'
                if si < len(path) and path[si] == '/':
                    memo[(pi, si)] = False
                    return False
                # Try matching zero or more characters
                # We can try matching 0, 1, 2, ... characters
                # But this can be O(n). We can optimize by trying all possibilities.
                # Option: match 0 chars: dp(pi + 1, si)
                # Option: match 1 char: dp(pi + 1, si + 1) if path[si] != '/'
                # ...
                # We can iterate
                for k in range(si, len(path) + 1):
                    if k > si and path[k-1] == '/':
                        break
                    if dp(pi + 1, k):
                        memo[(pi, si)] = True
                        return True
                memo[(pi, si)] = False
                return False
            else:
                # Literal character
                if si < len(path) and path[si] == p_char:
                    res = dp(pi + 1, si + 1)
                    memo[(pi, si)] = res
                    return res
                else:
                    memo[(pi, si)] = False
                    return False
        
        # If we get here, something is wrong
        memo[(pi, si)] = False
        return False
    
    return dp(0, 0)