def match(pattern, path):
    if not path.endswith('/') and pattern.endswith('/'):
        return False
    if path == '' and pattern == '':
        return True
    if path == '' and pattern == '**':
        return True
    if path == '' and pattern in ('*', '?', '[!]', '[!]', '[', ']'):
        return False
    if path == '' and (pattern.startswith('[') or pattern.startswith('?') or pattern.startswith('*')):
        if pattern == '**':
            return True
        if pattern.startswith('?') or pattern.startswith('*'):
            return False
        if pattern.startswith('['):
            return False
        return False

    pat_list = pattern.split('/')
    path_list = path.split('/')

    if len(pat_list) == 1:
        return match_segment(pat_list[0], path)

    # Handle ** specially
    new_pat = []
    for p in pat_list:
        if p == '**':
            if not new_pat or new_pat[-1] != '**':
                new_pat.append(p)
        else:
            new_pat.append(p)
    pat_list = new_pat

    # Check for consecutive ** which are redundant but handled by logic below
    # Actually, the spec says 'a/**/b'. So we can have multiple ** but they collapse logically.
    # Let's implement a recursive DP or iterative approach.

    m, n = len(pat_list), len(path_list)
    
    # dp[i][j] means pattern[:i] matches path[:j]
    # But ** allows skipping path segments.
    
    # Let's try a recursive approach with memoization
    memo = {}

    def solve(pi, pj):
        if (pi, pj) in memo:
            return memo[(pi, pj)]
        
        # Base cases
        if pi == m:
            res = pj == n
            memo[(pi, pj)] = res
            return res
        
        if pj == n:
            # Check if remaining pattern can match empty path
            # Only ** can match empty sequence of segments
            for k in range(pi, m):
                if pat_list[k] != '**':
                    res = False
                    memo[(pi, pj)] = res
                    return res
            res = True
            memo[(pi, pj)] = res
            return res

        p_seg = pat_list[pi]
        p_seg_next = pat_list[pi+1] if pi + 1 < m else None

        if p_seg == '**':
            # ** can match 0 or more segments
            # Option 1: match 0 segments -> skip this pattern segment, stay at same path segment
            if solve(pi + 1, pj):
                memo[(pi, pj)] = True
                return True
            # Option 2: match current path segment and try to match more with **
            if solve(pi, pj + 1):
                memo[(pi, pj)] = True
                return True
            memo[(pi, pj)] = False
            return False
        else:
            # Normal segment match
            if match_segment(p_seg, path_list[pj]):
                if solve(pi + 1, pj + 1):
                    memo[(pi, pj)] = True
                    return True
            memo[(pi, pj)] = False
            return False

    return solve(0, 0)

def match_segment(pattern, segment):
    if not pattern:
        return segment == ''
    
    # Parse pattern into tokens: char, ?, *, [..], \x
    # But since we can't use regex, we do manual parsing
    # Actually, we can just process char by char with backtracking
    
    p_idx = 0
    s_idx = 0
    p_len = len(pattern)
    s_len = len(segment)
    
    # We need to handle *, ?, [..], \x
    # Since * and ? and [..] are not allowed to match '/', and we are matching a single segment (no /),
    # we don't need to worry about / inside segment.
    
    # Use DP for segment matching
    # dp[i][j] = pattern[:i] matches segment[:j]
    dp = [[False] * (s_len + 1) for _ in range(p_len + 1)]
    dp[0][0] = True
    
    # Handle leading *
    for i in range(1, p_len + 1):
        if pattern[i-1] == '*':
            dp[i][0] = dp[i-1][0]
        elif pattern[i-1] == '\\':
            if i > 1:
                dp[i][0] = False # escaped char cannot match empty
            else:
                dp[i][0] = False
        else:
            dp[i][0] = False
    
    for i in range(1, p_len + 1):
        char_p = pattern[i-1]
        prev_char_p = pattern[i-2] if i > 1 else ''
        
        # Check if this is part of a character class
        if char_p == ']' and prev_char_p != '\\' and '[' in pattern[:i-1]:
             # This logic is complex to implement inline, better to parse tokens first
             pass

    # Let's rewrite match_segment with a helper to parse the pattern into tokens first
    tokens = tokenize_pattern(pattern)
    return match_tokens(tokens, segment)

def tokenize_pattern(pattern):
    tokens = []
    i = 0
    n = len(pattern)
    while i < n:
        c = pattern[i]
        if c == '\\':
            if i + 1 < n:
                tokens.append(('LIT', pattern[i+1]))
                i += 2
            else:
                tokens.append(('LIT', '\\'))
                i += 1
        elif c == '*':
            tokens.append(('STAR',))
            i += 1
        elif c == '?':
            tokens.append(('QMARK',))
            i += 1
        elif c == '[':
            j = i + 1
            neg = False
            if j < n and pattern[j] == '!':
                neg = True
                j += 1
            chars = set()
            while j < n and pattern[j] != ']':
                if j + 2 < n and pattern[j+1] == '-' and pattern[j+2] != ']':
                    start = pattern[j]
                    end = pattern[j+2]
                    if start <= end:
                        for k in range(ord(start), ord(end) + 1):
                            chars.add(chr(k))
                    j += 3
                else:
                    chars.add(pattern[j])
                    j += 1
            if j < n and pattern[j] == ']':
                tokens.append(('CLASS', chars, neg))
                i = j + 1
            else:
                # Invalid pattern, treat as literal? Or just fail?
                # Assuming valid input per spec
                tokens.append(('LIT', '['))
                i = j
        else:
            tokens.append(('LIT', c))
            i += 1
    return tokens

def match_tokens(tokens, segment):
    # DP: dp[i][j] = tokens[:i] matches segment[:j]
    n = len(tokens)
    m = len(segment)
    dp = [[False] * (m + 1) for _ in range(n + 1)]
    dp[0][0] = True
    
    # Handle leading *
    for i in range(1, n + 1):
        if tokens[i-1][0] == 'STAR':
            dp[i][0] = dp[i-1][0]
        else:
            break
            
    for i in range(1, n + 1):
        tok = tokens[i-1]
        for j in range(m + 1):
            if not dp[i-1][j]:
                continue
            
            if tok[0] == 'STAR':
                # * matches 0 or more chars
                dp[i][j] = dp[i][j] or dp[i-1][j] # match 0
                # match 1 or more
                if j < m:
                    dp[i][j+1] = dp[i][j+1] or dp[i][j] # This is wrong, * can match multiple
                    # Actually, standard * logic: dp[i][j] = dp[i-1][j] (match 0) or dp[i][j-1] (match more)
                    # But we are iterating j, so we need to be careful
                    pass
            
            elif tok[0] == 'QMARK':
                if j < m:
                    dp[i][j+1] = True
            elif tok[0] == 'LIT':
                if j < m and segment[j] == tok[1]:
                    dp[i][j+1] = True
            elif tok[0] == 'CLASS':
                chars, neg = tok[1], tok[2]
                if j < m:
                    c = segment[j]
                    in_class = c in chars
                    if (neg and not in_class) or (not neg and in_class):
                        dp[i][j+1] = True
    
    # Correct DP for STAR
    # Reset and redo properly
    dp = [[False] * (m + 1) for _ in range(n + 1)]
    dp[0][0] = True
    
    for i in range(1, n + 1):
        tok = tokens[i-1]
        for j in range(m + 1):
            if tok[0] == 'STAR':
                if dp[i-1][j]:
                    dp[i][j] = True
                if j < m and dp[i][j]:
                    dp[i][j+1] = True
            else:
                if j < m:
                    matched = False
                    if tok[0] == 'QMARK':
                        matched = True
                    elif tok[0] == 'LIT':
                        matched = (segment[j] == tok[1])
                    elif tok[0] == 'CLASS':
                        chars, neg = tok[1], tok[2]
                        in_class = segment[j] in chars
                        matched = (not neg and in_class) or (neg and not in_class)
                    
                    if matched and dp[i-1][j]:
                        dp[i][j+1] = True
    
    return dp[n][m]