```python
def match(pattern, path):
    """
    Returns True if a glob pattern matches a whole path.
    Paths are separated by '/'.
    '?' matches exactly one character but never '/'.
    '*' matches zero or more characters but never '/'.
    '**' matches zero or more whole segments.
    Square brackets are a character class.
    Backslash escapes the next character.
    """
    def parse_pattern(pat, index, path, path_index):
        # We'll use an iterative approach or recursion with memoization.
        # Given the constraints and complexity, let's use recursion with memoization.
        pass

    # Let's implement a recursive matcher with memoization
    # State: (pattern_index, path_index)
    memo = {}

    def is_match(pi, pxi):
        # pi: current index in pattern
        # pxi: current index in path
        
        state = (pi, pxi)
        if state in memo:
            return memo[state]
        
        plen = len(pattern)
        slen = len(path)
        
        # Base cases
        if pi == plen and pxi == slen:
            memo[state] = True
            return True
        
        # If path is exhausted but pattern remains, check if remaining pattern is all '*' or '**' separated by slashes
        if pxi == slen:
            # Check if remaining pattern can match empty string
            # This happens if remaining pattern is something like /* or /** or just **
            # Actually, '*' matches zero chars, '**' matches zero segments
            # We need to check if the rest of the pattern can match empty path
            i = pi
            while i < plen:
                if pattern[i] == '*':
                    # Skip consecutive stars
                    while i < plen and pattern[i] == '*':
                        i += 1
                    # After stars, we might have a '/' or end
                    if i < plen and pattern[i] == '/':
                        i += 1
                    continue
                else:
                    memo[state] = False
                    return False
            memo[state] = True
            return True
        
        # If pattern is exhausted but path remains
        if pi == plen:
            memo[state] = False
            return False
        
        # Check for '**'
        if pi + 1 < plen and pattern[pi] == '*' and pattern[pi+1] == '*':
            # '**' matches zero or more segments
            # Next char should be '/' or end of pattern
            # Case 1: '**/' at the end of pattern
            # Case 2: '**/' in the middle
            # Case 3: '**' at the end of pattern
            
            # Skip the '**'
            next_pi = pi + 2
            
            # Check if followed by '/'
            if next_pi < plen and pattern[next_pi] == '/':
                # '**/' matches zero or more segments followed by '/'
                # We can match zero segments: skip '**/' and try to match rest of pattern from current path position
                # We can match one or more segments: skip '**/' and advance path by one segment, then try again
                
                # Option 1: match zero segments (skip the '**/')
                if is_match(next_pi + 1, pxi):
                    memo[state] = True
                    return True
                
                # Option 2: match one or more segments (advance path past one segment, keep pattern)
                # Find the end of the current segment in path
                slash_pos = path.find('/', pxi)
                if slash_pos == -1:
                    # Rest of path is one segment
                    if is_match(next_pi + 1, slen):
                        memo[state] = True
                        return True
                else:
                    # Try advancing by one segment
                    if is_match(next_pi + 1, slash_pos + 1):
                        memo[state] = True
                        return True
                    # Also try matching just the segment without the slash? No, '**/' consumes the slash after the segments
                    # Actually, '**/' means we consume zero or more full segments. After consuming, we are at the '/' that follows the segments.
                    # So if we match k segments, we are at position after the k-th segment's slash.
                    # Let's re-think: '**/' can be seen as: skip any number of segments (including zero) until we find the pattern after '**/'
                    
                    # Let me redo this logic more carefully
                    pass
            else:
                # '**' at the end of pattern
                # Matches zero or more segments, consuming the rest of the path
                memo[state] = True
                return True
        
        # Check for '*'
        if pattern[pi] == '*':
            # '*' matches zero or more characters except '/'
            # Try matching 0, 1, 2, ... characters
            for k in range(slen - pxi + 1):
                # Check if path[pxi : pxi+k] contains no '/'
                segment = path[pxi:pxi+k]
                if '/' in segment:
                    break
                if is_match(pi + 1, pxi + k):
                    memo[state] = True
                    return True
            memo[state] = False
            return False
        
        # Check for character class [...]
        if pattern[pi] == '[':
            # Find the closing bracket
            j = pi + 1
            if j < plen and pattern[j] == '!':
                j += 1
            if j < plen and pattern[j] == ']':
                j += 1
            while j < plen and pattern[j] != ']':
                j += 1
            if j >= plen:
                # No closing bracket, treat '[' as literal
                if path[pxi] == pattern[pi]:
                    res = is_match(pi + 1, pxi + 1)
                    memo[state] = res
                    return res
                else:
                    memo[state] = False
                    return False
            
            # Extract the class content
            class_content = pattern[pi+1:j]
            # Check for negation
            negated = False
            idx = 0
            if class_content[0] == '!':
                negated = True
                idx = 1
            
            char = path[pxi]
            
            # Check if char matches the class
            matched = False
            if char == '/':
                matched = False
            else:
                # Parse the class content
                i = idx
                clen = len(class_content)
                while i < clen:
                    if i + 2 < clen and class_content[i+1] == '-' and class_content[i+2] != ']':
                        # Range
                        start_char = class_content[i]
                        end_char = class_content[i+2]
                        if start_char <= char <= end_char:
                            matched = True
                            break
                        i += 3
                    else:
                        if class_content[i] == char:
                            matched = True
                            break
                        i += 1
            
            if negated:
                matched = not matched
            
            if matched:
                res = is_match(j + 1, pxi + 1)
                memo[state] = res
                return res
            else:
                memo[state] = False
                return False
        
        # Check for escaped character
        if pattern[pi] == '\\':
            if pi + 1 < plen:
                literal = pattern[pi + 1]
                if pxi < slen and path[pxi] == literal:
                    res = is_match(pi + 2, pxi + 1)
                    memo[state] = res
                    return res
                else:
                    memo[state] = False
                    return False
            else:
                # Trailing backslash, treat as literal backslash? Or error?
                # Let's treat as literal
                if pxi < slen and path[pxi] == '\\':
                    res = is_match(pi + 1, pxi + 1)
                    memo[state] = res
                    return res
                else:
                    memo[state] = False
                    return False
        
        # Regular character match
        if path[pxi] == pattern[pi]:
            res = is_match(pi + 1, pxi + 1)
            memo[state] = res
            return res
        else:
            memo[state] = False
            return False
    
    return is_match(0, 0)