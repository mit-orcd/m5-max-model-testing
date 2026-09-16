def match(pattern, path):
    def parse_pattern(p):
        parts = []
        i = 0
        while i < len(p):
            if p[i] == '\\':
                if i + 1 < len(p):
                    parts.append(('lit', p[i+1]))
                    i += 2
                else:
                    parts.append(('lit', '\\'))
                    i += 1
            elif p[i] == '*':
                if i + 1 < len(p) and p[i+1] == '*':
                    parts.append(('glob_seg', '**'))
                    i += 2
                else:
                    parts.append(('glob_char', '*'))
                    i += 1
            elif p[i] == '?':
                parts.append(('glob_char', '?'))
                i += 1
            elif p[i] == '[':
                i += 1
                negate = False
                if i < len(p) and p[i] == '!':
                    negate = True
                    i += 1
                chars = set()
                ranges = []
                while i < len(p) and p[i] != ']':
                    if i + 2 < len(p) and p[i+1] == '-' and p[i+2] != ']':
                        ranges.append((p[i], p[i+2]))
                        i += 3
                    else:
                        chars.add(p[i])
                        i += 1
                if i < len(p) and p[i] == ']':
                    i += 1
                parts.append(('class', negate, chars, ranges))
            elif p[i] == '/':
                parts.append(('sep', '/'))
                i += 1
            else:
                parts.append(('lit', p[i]))
                i += 1
        return parts

    def match_char(p_type, p_val, char):
        if char == '/': return False
        if p_type == 'lit': return char == p_val
        if p_type == 'glob_char':
            return p_val == '?' or p_val == '*'
        if p_type == 'class':
            negate, chars, ranges = p_val
            found = char in chars
            if not found:
                for start, end in ranges:
                    if start <= char <= end:
                        found = True
                        break
            return not found if negate else found
        return False

    p_tokens = parse_pattern(pattern)
    path_segments = path.split('/')
    
    # We need to handle the pattern as a sequence of segments or characters.
    # However, '**' is a segment-level wildcard. 
    # Let's re-parse pattern into a list of segment-level patterns.
    
    seg_patterns = []
    curr_seg = []
    i = 0
    while i < len(p_tokens):
        t = p_tokens[i]
        if t[0] == 'sep':
            seg_patterns.append(curr_seg)
            curr_seg = []
        else:
            curr_seg.append(t)
        i += 1
    seg_patterns.append(curr_seg)

    # If pattern ends with '/', split produces an extra empty segment.
    # But 'a/' in glob usually means path 'a/'.
    # path.split('/') for 'a/' is ['a', '']
    # if pattern is 'a/', seg_patterns is [['lit','a'], []]
    
    memo = {}

    def solve(seg_idx, pat_idx):
        state = (seg_idx, pat_idx)
        if state in memo: return memo[state]

        # Base case: reached end of pattern segments
        if pat_idx == len(seg_patterns):
            return seg_idx == len(path_segments)

        # Handle ** segment pattern
        # Check if current pattern segment is exactly '**'
        is_double_star = (len(seg_patterns[pat_idx]) == 1 and 
                          seg_patterns[pat_idx][0][0] == 'glob_seg' and 
                          seg_patterns[pat_idx][0][1] == '**')

        if is_double_star:
            # Try matching zero or more segments
            for k in range(seg_idx, len(path_segments) + 1):
                if solve(k, pat_idx + 1):
                    memo[state] = True
                    return True
            memo[state] = False
            return False

        # Standard segment matching
        if seg_idx >= len(path_segments):
            # Pattern expects a segment, but path is exhausted
            # This only works if the pattern segment is empty and we are matching empty strings?
            # No, segments are separated by '/'.
            # If pattern is 'a/', seg_patterns is [['a'], []]. path 'a/' -> ['a', ''].
            # If pattern is 'a', seg_patterns is [['a']]. path 'a' -> ['a'].
            # If pattern is 'a/', seg_patterns is [['a'], []]. path 'a' -> ['a'].
            # Wait, if pattern is 'a/', it must match 'a/'.
            # Let's refine: a pattern 'a/' matches path 'a/'. 
            # path.split('/') for 'a/' is ['a', '']. 
            # seg_patterns for 'a/' is [['a'], []].
            # So we check if the remaining segments match the remaining pattern segments.
            # But we must handle the case where pattern expects a segment and path doesn't have it.
            # The only way is if the pattern segment is empty (e.g. trailing slash).
            if len(seg_patterns[pat_idx]) == 0:
                 # Empty segment pattern (like trailing slash)
                 # This matches an empty string segment
                 if seg_idx < len(path_segments) and path_segments[seg_idx] == '':
                     res = solve(seg_idx + 1, pat_idx + 1)
                     memo[state] = res
                     return res
                 elif seg_idx == len(path_segments):
                     # This part is tricky. 'a/' pattern vs 'a' path.
                     # Usually 'a/' does not match 'a'.
                     # If pattern is 'a/', seg_patterns is [['a'], []]. path 'a' is ['a'].
                     # solve(0,0) -> matches 'a', then solve(1,1). 
                     # solve(1,1) -> pat_idx 1 is []. seg_idx 1 is len(path_segments).
                     # solve(1,1) returns True.
                     # But 'a/' should not match 'a'.
                     # Let's adjust: if pattern has more segments, path must have more.
                     # Unless the pattern segment is empty and we are at the end.
                     # Actually, 'a/' split is ['a', '']. 'a' split is ['a'].
                     # So 'a/' requires a second segment which is empty.
                     pass 
            memo[state] = False
            return False

        # Match current segment path_segments[seg_idx] with seg_patterns[pat_idx]
        target = path_segments[seg_idx]
        p_seg = seg_patterns[pat_idx]
        
        # Match the segment string
        def match_segment(s, tokens):
            if not tokens: return len(s) == 0
            # Token-based matching for a single segment
            # This is a mini-DP or recursive match
            m_memo = {}
            def m_solve(s_idx, t_idx):
                if (s_idx, t_idx) in m_memo: return m_memo[(s_idx, t_idx)]
                if t_idx == len(tokens):
                    return s_idx == len(s)
                
                token = tokens[t_idx]
                if token[0] == 'glob_char':
                    if token[1] == '*':
                        # '*' in a segment matches zero or more characters
                        for k in range(s_idx, len(s) + 1):
                            if m_solve(k, t_idx + 1):
                                m_memo[(s_idx, t_idx)] = True
                                return True
                        m_memo[(s_idx, t_idx)] = False
                        return False
                    else: # '?'
                        if s_idx < len(s) and match_char(token[0], token[1], s[s_idx]):
                            res = m_solve(s_idx + 1, t_idx + 1)
                            m_memo[(s_idx, t_idx)] = res
                            return res
                        m_memo[(s_idx, t_idx)] = False
                        return False
                else:
                    if s_idx < len(s) and match_char(token[0], token[1], s[s_idx]):
                        res = m_solve(s_idx + 1, t_idx + 1)
                        m_memo[(s_idx, t_idx)] = res
                        return res
                    m_memo[(s_idx, t_idx)] = False
                    return False

            return m_solve(0, 0)

        if match_segment(target, p_seg):
            res = solve(seg_idx + 1, pat_idx + 1)
            memo[state] = res
            return res
        
        memo[state] = False
        return False

    # Special handling for trailing slash in pattern:
    # 'a/' -> seg_patterns: [['a'], []]. path 'a/' -> path_segments: ['a', ''].
    # 'a' -> seg_patterns: [['a']]. path 'a' -> path_segments: ['a'].
    # The split logic is consistent.
    return solve(0, 0)