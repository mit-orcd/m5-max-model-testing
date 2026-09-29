```python
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
                    parts.append(('glob_star', None))
                    i += 2
                else:
                    parts.append(('glob_single', None))
                    i += 1
            elif p[i] == '?':
                parts.append(('qmark', None))
                i += 1
            elif p[i] == '[':
                i += 1
                negate = False
                if i < len(p) and p[i] == '!':
                    negate = True
                    i += 1
                chars = []
                while i < len(p) and p[i] != ']':
                    if i + 2 < len(p) and p[i+1] == '-' and p[i+2] != ']':
                        chars.append(('range', p[i], p[i+2]))
                        i += 3
                    else:
                        chars.append(('char', p[i]))
                        i += 1
                if i < len(p) and p[i] == ']':
                    i += 1
                parts.append(('class', (negate, chars)))
            elif p[i] == '/':
                parts.append(('sep', '/'))
                i += 1
            else:
                parts.append(('lit', p[i]))
                i += 1
        return parts

    def match_class(cls, char):
        if char == '/': return False
        negate, items = cls
        found = False
        for item in items:
            if item[0] == 'char':
                if item[1] == char:
                    found = True; break
            elif item[0] == 'range':
                if item[1] <= char <= item[2]:
                    found = True; break
        return not found if negate else found

    pattern_parts = parse_pattern(pattern)
    path_segments = path.split('/')
    
    # Special handling for '**' which matches zero or more segments.
    # We convert the pattern into a list of segments where each segment is a list of pattern tokens.
    seg_patterns = []
    current_seg = []
    for p in pattern_parts:
        if p[0] == 'sep':
            seg_patterns.append(current_seg)
            current_seg = []
        else:
            current_seg.append(p)
    seg_patterns.append(current_seg)

    # Handle the '**' logic. '**' acts as a segment-level wildcard.
    # We need to find where '**' is in the pattern segments.
    # A pattern like ['a', '**', 'b'] means segment 'a', then 0+ segments, then segment 'b'.
    
    memo = {}

    def solve(p_idx, s_idx):
        state = (p_idx, s_idx)
        if state in memo: return memo[state]

        if p_idx == len(seg_patterns):
            return s_idx == len(path_segments)

        # Check if current pattern segment is '**'
        is_double_star = len(seg_patterns[p_idx]) == 1 and seg_patterns[p_idx][0][0] == 'glob_star'
        
        if is_double_star:
            # Try matching 0, 1, 2... segments
            for k in range(s_idx, len(path_segments) + 1):
                if solve(p_idx + 1, k):
                    memo[state] = True
                    return True
            memo[state] = False
            return False
        
        # If pattern segment is empty (e.g. pattern starts with / or ends with /)
        # and we are at a segment boundary.
        if not seg_patterns[p_idx]:
            # If pattern is empty segment, it must match an empty path segment or boundary.
            # In our split logic, '/' results in ['', ''] for '/'. 
            # A pattern segment is empty if it's just a separator.
            # If pattern is 'a/', segments are ['a', '']
            if s_idx < len(path_segments) and seg_patterns[p_idx] == [] and p_idx < len(seg_patterns)-1:
                # This is tricky. Let's refine.
                pass

        # Standard segment matching
        # If pattern segment is empty (caused by leading/trailing/double slashes)
        if not seg_patterns[p_idx]:
            # This occurs if pattern has '/' at start, end, or '//'
            # If pattern is '/', seg_patterns is [[], []]. path '/' is ['', '']
            # We must match the empty string in path_segments.
            if s_idx < len(path_segments):
                if path_segments[s_idx] == '':
                    if solve(p_idx + 1, s_idx + 1):
                        memo[state] = True
                        return True
            memo[state] = False
            return False

        # Match the current segment path_segments[s_idx] with seg_patterns[p_idx]
        if s_idx >= len(path_segments):
            memo[state] = False
            return False

        # Match segment using backtracking for '*' within segment
        def match_seg(tokens, segment_str, t_idx, char_idx):
            if t_idx == len(tokens):
                return char_idx == len(segment_str)
            
            token = tokens[t_idx]
            if token[0] == 'lit':
                if char_idx < len(segment_str) and segment_str[char_idx] == token[1]:
                    return match_seg(tokens, segment_str, t_idx + 1, char_idx + 1)
                return False
            elif token[0] == 'qmark':
                if char_idx < len(segment_str) and segment_str[char_idx] != '/':
                    return match_seg(tokens, segment_str, t_idx + 1, char_idx + 1)
                return False
            elif token[0] == 'glob_single':
                if char_idx < len(segment_str) and segment_str[char_idx] != '/':
                    return match_seg(tokens, segment_str, t_idx + 1, char_idx + 1)
                return False
            elif token[0] == 'class':
                if char_idx < len(segment_str) and match_class(token[1], segment_str[char_idx]):
                    return match_seg(tokens, segment_str, t_idx + 1, char_idx + 1)
                return False
            elif token[0] == 'glob_star':
                # '*' within a segment (not '**')
                for end in range(char_idx, len(segment_str) + 1):
                    if match_seg(tokens, segment_str, t_idx + 1, end):
                        return True
                return False
            return False

        if match_seg(seg_patterns[p_idx], path_segments[s_idx], 0, 0):
            if solve(p_idx + 1, s_idx + 1):
                memo[state] = True
                return True

        memo[state] = False
        return False

    # Special case: pattern '/' -> segments ['', ''], path '/' -> segments ['', '']
    # The logic above handles segments.
    return solve(0,