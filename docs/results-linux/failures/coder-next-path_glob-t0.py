def match(pattern, path):
    def match_segment(p_seg, seg):
        # p_seg is a pattern segment, seg is a path segment
        pi = 0
        si = 0
        while pi < len(p_seg) or si < len(seg):
            if pi < len(p_seg):
                c = p_seg[pi]
                if c == '\\':
                    if pi + 1 < len(p_seg):
                        pi += 1
                        if si >= len(seg) or p_seg[pi] != seg[si]:
                            return False
                        pi += 1
                        si += 1
                        continue
                    else:
                        # Trailing backslash, treat as literal
                        if si >= len(seg) or '\\' != seg[si]:
                            return False
                        pi += 1
                        si += 1
                        continue
                elif c == '?':
                    if si >= len(seg) or seg[si] == '/':
                        return False
                    pi += 1
                    si += 1
                    continue
                elif c == '*':
                    # '*' matches zero or more non-slash characters
                    # Try matching 0, 1, 2, ... characters
                    pi += 1
                    # Try all possible lengths
                    for i in range(len(seg) - si + 1):
                        if match_segment(p_seg[pi:], seg[si + i:]):
                            return True
                    return False
                elif c == '[':
                    # Character class
                    pi += 1
                    if si >= len(seg) or seg[si] == '/':
                        return False
                    neg = False
                    if pi < len(p_seg) and p_seg[pi] == '!':
                        neg = True
                        pi += 1
                    # Find closing bracket
                    bracket_end = pi
                    if bracket_end < len(p_seg) and p_seg[bracket_end] == ']':
                        bracket_end += 1
                    while bracket_end < len(p_seg) and p_seg[bracket_end] != ']':
                        bracket_end += 1
                    if bracket_end >= len(p_seg):
                        # Malformed, treat as literal '['
                        if seg[si] != '[':
                            return False
                        pi += 1
                        si += 1
                        continue
                    # Parse character class
                    matched = False
                    pi += 1
                    while pi < bracket_end:
                        if pi + 2 < bracket_end and p_seg[pi + 1] == '-' and p_seg[pi + 2] != ']':
                            # Range
                            start = p_seg[pi]
                            end = p_seg[pi + 2]
                            pi += 3
                            if start <= seg[si] <= end:
                                matched = True
                                break
                        else:
                            # Single character
                            ch = p_seg[pi]
                            pi += 1
                            if ch == seg[si]:
                                matched = True
                                break
                    if neg:
                        matched = not matched
                    if not matched:
                        return False
                    pi = bracket_end + 1
                    si += 1
                    continue
                else:
                    # Literal character
                    if si >= len(seg) or c != seg[si]:
                        return False
                    pi += 1
                    si += 1
                    continue
            else:
                return False
        return pi == len(p_seg) and si == len(seg)
    
    # Split pattern and path by '/'
    p_parts = []
    i = 0
    while i < len(pattern):
        if pattern[i] == '\\':
            if i + 1 < len(pattern):
                p_parts.append(pattern[i+1])
                i += 2
                continue
            else:
                p_parts.append('\\')
                i += 1
                continue
        elif pattern[i] == '/':
            p_parts.append('')
            i += 1
            continue
        else:
            # Collect segment until next '/' or end
            j = i
            while j < len(pattern) and pattern[j] != '/':
                j += 1
            p_parts.append(pattern[i:j])
            i = j
    
    # Handle empty pattern
    if not p_parts:
        return path == ''
    
    # Split path by '/'
    path_parts = path.split('/') if path else ['']
    
    # Handle '**' specially
    # We'll use dynamic programming: dp[i][j] = True if p_parts[i:] matches path_parts[j:]
    n = len(p_parts)
    m = len(path_parts)
    
    # dp[i][j] = whether p_parts[i:] matches path_parts[j:]
    dp = [[False] * (m + 1) for _ in range(n + 1)]
    dp[n][m] = True  # empty pattern matches empty path
    
    # Fill dp from bottom-right
    for i in range(n - 1, -1, -1):
        for j in range(m, -1, -1):
            if p_parts[i] == '**':
                # '**' matches zero or more segments
                # Option 1: match zero segments (skip '**')
                if dp[i + 1][j]:
                    dp[i][j] = True
                # Option 2: match one or more segments (consume one segment and keep '**')
                elif j < m and dp[i][j + 1]:
                    dp[i][j] = True
            else:
                if j < m:
                    # Try matching p_parts[i] with path_parts[j]
                    if match_segment(p_parts[i], path_parts[j]):
                        dp[i][j] = dp[i + 1][j + 1]
    
    return dp[0][0]