def match(pattern, path):
    # Tokenize pattern into a list of tokens
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
            if i + 1 < n and pattern[i+1] == '*':
                tokens.append(('STAR_STAR',))
                i += 2
            else:
                tokens.append(('STAR',))
                i += 1
        elif c == '?':
            tokens.append(('QMARK',))
            i += 1
        elif c == '[':
            j = i + 1
            negated = False
            if j < n and pattern[j] == '!':
                negated = True
                j += 1
            if j >= n:
                tokens.append(('LIT', '['))
                i += 1
                continue
            chars = set()
            while j < n and pattern[j] != ']':
                if pattern[j] == '\\' and j + 1 < n:
                    chars.add(pattern[j+1])
                    j += 2
                elif j + 2 < n and pattern[j+1] == '-' and pattern[j+2] != ']':
                    start, end = pattern[j], pattern[j+2]
                    if ord(start) > ord(end):
                        start, end = end, start
                    for k in range(ord(start), ord(end) + 1):
                        chars.add(chr(k))
                    j += 3
                else:
                    chars.add(pattern[j])
                    j += 1
            if j < n:
                tokens.append(('CLASS', chars, negated))
                i = j + 1
            else:
                tokens.append(('LIT', '['))
                i += 1
        else:
            tokens.append(('LIT', c))
            i += 1

    segments_path = path.split('/')
    segments_pat = []
    i = 0
    n_tokens = len(tokens)
    while i < n_tokens:
        if tokens[i][0] == 'STAR_STAR':
            segments_pat.append(('STAR_STAR',))
            i += 1
        else:
            seg_tokens = []
            while i < n_tokens and tokens[i][0] != 'STAR_STAR':
                seg_tokens.append(tokens[i])
                i += 1
            if seg_tokens:
                segments_pat.append(seg_tokens)
    
    # DP table: dp[i][j] = True if first i segments of path match first j segments of pattern
    m = len(segments_path)
    n_seg_pat = len(segments_pat)
    
    # Precompute if a segment matches a list of tokens
    def match_segment(s, seg_tokens):
        if not seg_tokens:
            return len(s) == 0
        if len(seg_tokens) == 1 and seg_tokens[0][0] == 'STAR':
            # '*' matches zero or more non-slash chars
            for c in s:
                if c == '/': return False
            return True
        # Match token by token
        ti = 0
        si = 0
        while si < len(s) and ti < len(seg_tokens):
            t = seg_tokens[ti]
            if t[0] == 'LIT':
                if si >= len(s) or s[si] != t[1]:
                    return False
                si += 1
                ti += 1
            elif t[0] == 'QMARK':
                if si >= len(s) or s[si] == '/':
                    return False
                si += 1
                ti += 1
            elif t[0] == 'CLASS':
                chars, negated = t[1], t[2]
                if si >= len(s) or s[si] == '/':
                    return False
                in_class = s[si] in chars
                if negated:
                    if in_class: return False
                else:
                    if not in_class: return False
                si += 1
                ti += 1
            elif t[0] == 'STAR':
                # '*' matches zero or more non-slash
                # Greedy match
                count = 0
                while si + count < len(s) and s[si+count] != '/':
                    count += 1
                # Try all possibilities
                for k in range(count + 1):
                    if match_segment(s[si+si+k:], seg_tokens[ti+1:]): # This is wrong logic, need recursive helper
                        pass
                # Actually, better to do a loop inside
                # Let's restructure: match_segment should handle the rest of tokens recursively or iteratively
                pass
        # This approach for match_segment is flawed. Let's rewrite the matching logic properly.
        return False

    # Rewrite matching logic with a cleaner function
    def is_match_segment(segment, tokens_list):
        if not tokens_list:
            return len(segment) == 0
        # Check if tokens_list starts with '*'
        if len(tokens_list) == 1 and tokens_list[0][0] == 'STAR':
            return '/' not in segment
        # Otherwise, match character by character
        ti = 0
        si = 0
        n_t = len(tokens_list)
        n_s = len(segment)
        
        # We need to handle '*' which can match multiple chars
        # Use a stack or recursion for '*' expansion? Or iterative with backtracking?
        # Since '*' is greedy but can backtrack, we can use a simple loop with state
        # But simpler: convert '*' into a state machine
        # Actually, let's just do a recursive match with memoization for this segment
        memo = {}
        def match_rec(idx_t, idx_s):
            if (idx_t, idx_s) in memo:
                return memo[(idx_t, idx_s)]
            if idx_t == n_t:
                res = idx_s == n_s
                memo[(idx_t, idx_s)] = res
                return res
            t = tokens_list[idx_t]
            if t[0] == 'STAR':
                # Match 0 or more chars
                # Try matching 0 chars
                if match_rec(idx_t + 1, idx_s):
                    memo[(idx_t, idx_s)] = True
                    return True
                # Try matching 1 or more
                if idx_s < n_s and segment[idx_s] != '/':
                    if match_rec(idx_t, idx_s + 1):
                        memo[(idx_t, idx_s)] = True
                        return True
                memo[(idx_t, idx_s)] = False
                return False
            elif t[0] == 'LIT':
                if idx_s < n_s and segment[idx_s] == t[1]:
                    res = match_rec(idx_t + 1, idx_s + 1)
                    memo[(idx_t, idx_s)] = res
                    return res
                memo[(idx_t, idx_s)] = False
                return False
            elif t[0] == 'QMARK':
                if idx_s < n_s and segment[idx_s] != '/':
                    res = match_rec(idx_t + 1, idx_s + 1)
                    memo[(idx_t, idx_s)] = res
                    return res
                memo[(idx_t, idx_s)] = False
                return False
            elif t[0] == 'CLASS':
                chars, negated = t[1], t[2]
                if idx_s < n_s and segment[idx_s] != '/':
                    ch = segment[idx_s]
                    in_class = ch in chars
                    if (negated and not in_class) or (not negated and in_class):
                        res = match_rec(idx_t + 1, idx_s + 1)
                        memo[(idx_t, idx_s)] = res
                        return res
                memo[(idx_t, idx_s)] = False
                return False
            return False
        
        return match_rec(0, 0)

    # DP for segments
    # dp[i][j] = True if first i path segments match first j pattern segments
    dp = [[False] * (n_seg_pat + 1) for _ in range(m + 1)]
    dp[0][0] = True

    for i in range(m + 1):
        for j in range(n_seg_pat + 1):
            if i == 0 and j == 0:
                continue
            if j == 0:
                dp[i][j] = False
                continue
            
            p_seg = segments_pat[j-1]
            
            if p_seg[0][0] == 'STAR_STAR':
                # '**' matches zero or more segments
                # If we match 0 segments: dp[i][j] = dp[i][j-1]
                if dp[i][j-1]:
                    dp[i][j] = True
                    continue
                # If we match 1 or more segments: dp[i][j] = dp[i-1][j]
                if i > 0 and dp[i-1][j]:
                    dp[i][j] = True
                    continue
            else:
                # Regular segment
                if i > 0:
                    if is_match_segment(segments_path[i-1], p_seg):
                        if dp[i-1][j-1]:
                            dp[i][j] = True
    
    return dp[m][n_seg_pat]