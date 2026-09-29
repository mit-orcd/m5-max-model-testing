def match(pattern, path):
    def compile_pattern(p):
        tokens = []
        i = 0
        n = len(p)
        while i < n:
            c = p[i]
            if c == '\\':
                if i + 1 < n:
                    tokens.append(('lit', p[i + 1]))
                    i += 2
                else:
                    tokens.append(('lit', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('qmark',))
                i += 1
            elif c == '*':
                if i + 1 < n and p[i + 1] == '*':
                    tokens.append(('starstar',))
                    i += 2
                else:
                    tokens.append(('star',))
                    i += 1
            elif c == '[':
                j = i + 1
                neg = False
                if j < n and p[j] == '!':
                    neg = True
                    j += 1
                if j < n and p[j] == ']':
                    j += 1
                chars = set()
                while j < n and p[j] != ']':
                    if p[j] == '\\' and j + 1 < n:
                        chars.add(p[j + 1])
                        j += 2
                    elif j + 2 < n and p[j + 1] == '-' and p[j + 2] != ']':
                        chars.update(range(ord(p[j]), ord(p[j + 2]) + 1))
                        j += 3
                    else:
                        chars.add(p[j])
                        j += 1
                if j >= n:
                    tokens.append(('lit', '['))
                    i += 1
                else:
                    tokens.append(('class', chars, neg))
                    i = j + 1
            else:
                tokens.append(('lit', c))
                i += 1
        return tokens

    def match_segment(seg_tokens, seg):
        if not seg_tokens:
            return seg == ''
        if not seg:
            return False
        i = 0
        j = 0
        n = len(seg_tokens)
        m = len(seg)
        while i < n and j < m:
            tok = seg_tokens[i]
            if tok[0] == 'lit':
                if tok[1] != seg[j]:
                    return False
                i += 1
                j += 1
            elif tok[0] == 'qmark':
                i += 1
                j += 1
            elif tok[0] == 'class':
                chars, neg = tok[1], tok[2]
                if neg:
                    if seg[j] in chars:
                        return False
                else:
                    if seg[j] not in chars:
                        return False
                i += 1
                j += 1
            elif tok[0] == 'star':
                # '*' matches zero or more chars in segment
                # Try matching zero first, then one, etc.
                # But since we are in a segment, we can use simple recursion or DP
                # Here we use a simple backtracking approach for the segment
                # Actually, for a single segment with only stars, we can do:
                # But we have mixed tokens. Let's use a simple recursive helper for segment matching
                return match_segment_recursive(seg_tokens[i:], seg[j:])
        return i == n and j == m

    def match_segment_recursive(tokens, s):
        if not tokens:
            return not s
        if not s:
            # Check if remaining tokens can match empty string
            # Only stars can match empty
            for t in tokens:
                if t[0] != 'star':
                    return False
            return True
        tok = tokens[0]
        if tok[0] == 'lit':
            if tok[1] != s[0]:
                return False
            return match_segment_recursive(tokens[1:], s[1:])
        elif tok[0] == 'qmark':
            return match_segment_recursive(tokens[1:], s[1:])
        elif tok[0] == 'class':
            chars, neg = tok[1], tok[2]
            if neg:
                if s[0] in chars:
                    return False
            else:
                if s[0] not in chars:
                    return False
            return match_segment_recursive(tokens[1:], s[1:])
        elif tok[0] == 'star':
            # Try matching zero or more characters
            # Option 1: match zero characters
            if match_segment_recursive(tokens[1:], s):
                return True
            # Option 2: match one or more characters
            return match_segment_recursive(tokens, s[1:])
        return False

    def match_path(pattern_tokens, path_segments):
        if not pattern_tokens:
            return not path_segments
        if not path_segments:
            # Check if remaining pattern can match empty path
            # Only starstar can match empty segments
            for t in pattern_tokens:
                if t[0] != 'starstar':
                    return False
            return True

        tok = pattern_tokens[0]
        if tok[0] == 'starstar':
            # '**' matches zero or more segments
            # Option 1: match zero segments
            if match_path(pattern_tokens[1:], path_segments):
                return True
            # Option 2: match one or more segments
            return match_path(pattern_tokens, path_segments[1:])
        elif tok[0] == 'star':
            # '*' matches zero or more characters in current segment
            # But it cannot cross segments. So it must match within the first segment.
            # However, the pattern token 'star' is for a single segment.
            # So we need to match the current segment with a pattern that starts with 'star'
            # But our pattern_tokens are for the whole path. We need to split the pattern by segments.
            # Actually, we should have compiled the pattern into segment patterns.
            # Let's change approach: compile pattern into list of segment patterns.
            return False  # This case should not happen if we compile correctly
        else:
            # Literal, qmark, class: must match current segment
            # But we need to handle the case where the pattern has multiple tokens for one segment
            # So we need to group tokens by segment
            return False  # This case should not happen if we compile correctly

    # Better approach: compile pattern into a list of segment patterns
    def compile_pattern_to_segments(p):
        segments = []
        current_seg = []
        i = 0
        n = len(p)
        while i < n:
            c = p[i]
            if c == '/':
                if current_seg:
                    segments.append(current_seg)
                    current_seg = []
                i += 1
            elif c == '\\':
                if i + 1 < n:
                    current_seg.append(('lit', p[i + 1]))
                    i += 2
                else:
                    current_seg.append(('lit', '\\'))
                    i += 1
            elif c == '?':
                current_seg.append(('qmark',))
                i += 1
            elif c == '*':
                if i + 1 < n and p[i + 1] == '*':
                    # '**' is a special token that spans segments
                    # We'll represent it as a special token in the segment list
                    # But we need to handle it separately
                    # Let's push a special token for '**'
                    segments.append(('starstar',))
                    i += 2
                    # Skip the next '/' if present
                    if i < n and p[i] == '/':
                        i += 1
                else:
                    current_seg.append(('star',))
                    i += 1
            elif c == '[':
                j = i + 1
                neg = False
                if j < n and p[j] == '!':
                    neg = True
                    j += 1
                if j < n and p[j] == ']':
                    j += 1
                chars = set()
                while j < n and p[j] != ']':
                    if p[j] == '\\' and j + 1 < n:
                        chars.add(p[j + 1])
                        j += 2
                    elif j + 2 < n and p[j + 1] == '-' and p[j + 2] != ']':
                        chars.update(range(ord(p[j]), ord(p[j + 2]) + 1))
                        j += 3
                    else:
                        chars.add(p[j])
                        j += 1
                if j >= n:
                    current_seg.append(('lit', '['))
                    i += 1
                else:
                    current_seg.append(('class', chars, neg))
                    i = j + 1
            else:
                current_seg.append(('lit', c))
                i += 1
        if current_seg:
            segments.append(current_seg)
        return segments

    def match_segment_pattern(seg_tokens, seg):
        if not seg_tokens:
            return seg == ''
        if not seg:
            return False
        i = 0
        j = 0
        n = len(seg_tokens)
        m = len(seg)
        while i < n and j < m:
            tok = seg_tokens[i]
            if tok[0] == 'lit':
                if tok[1] != seg[j]:
                    return False
                i += 1
                j += 1
            elif tok[0] == 'qmark':
                i += 1
                j += 1
            elif tok[0] == 'class':
                chars, neg = tok[1], tok[2]
                if neg:
                    if seg[j] in chars:
                        return False
                else:
                    if seg[j] not in chars:
                        return False
                i += 1
                j += 1
            elif tok[0] == 'star':
                # '*' matches zero or more characters in segment
                # Use recursive matching for the rest of the segment
                return match_segment_recursive(seg_tokens[i:], seg[j:])
        return i == n and j == m

    def match_segment_recursive(tokens, s):
        if not tokens:
            return not s
        if not s:
            for t in tokens:
                if t[0] != 'star':
                    return False
            return True
        tok = tokens[0]
        if tok[0] == 'lit':
            if tok[1] != s[0]:
                return False
            return match_segment_recursive(tokens[1:], s[1:])
        elif tok[0] == 'qmark':
            return match_segment_recursive(tokens[1:], s[1:])
        elif tok[0] == 'class':
            chars, neg = tok[1], tok[2]
            if neg:
                if s[0] in chars:
                    return False
            else:
                if s[0] not in chars:
                    return False
            return match_segment_recursive(tokens[1:], s[1:])
        elif tok[0] == 'star':
            if match_segment_recursive(tokens[1:], s):
                return True
            return match_segment_recursive(tokens, s[1:])
        return False

    def match_path_segments(pattern_segs, path_segs):
        if not pattern_segs:
            return not path_segs
        if not path_segs:
            for seg in pattern_segs:
                if seg != ('starstar',):
                    return False
            return True

        tok = pattern_segs[0]
        if tok == ('starstar',):
            # Match zero or more segments
            if match_path_segments(pattern_segs[1:], path_segs):
                return True
            return match_path_segments(pattern_segs, path_segs[1:])
        else:
            # tok is a list of tokens for a segment
            if not path_segs:
                return False
            if not match_segment_pattern(tok, path_segs[0]):
                return False
            return match_path_segments(pattern_segs[1:], path_segs[1:])

    pattern_segs = compile_pattern_to_segments(pattern)
    path_segs = path.split('/') if path else []
    return match_path_segments(pattern_segs, path_segs)