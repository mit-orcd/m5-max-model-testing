```python
def match(pattern, path):
    def parse_pattern(pat):
        tokens = []
        i = 0
        while i < len(pat):
            c = pat[i]
            if c == '\\':
                if i + 1 < len(pat):
                    tokens.append(('literal', pat[i + 1]))
                    i += 2
                else:
                    tokens.append(('literal', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('question',))
                i += 1
            elif c == '*':
                if i + 1 < len(pat) and pat[i + 1] == '*':
                    # '**' - match zero or more whole segments
                    tokens.append(('starstar',))
                    i += 2
                    # consume any additional '*'
                    while i < len(pat) and pat[i] == '*':
                        i += 1
                else:
                    tokens.append(('star',))
                    i += 1
            elif c == '[':
                j = i + 1
                negated = False
                if j < len(pat) and pat[j] == '!':
                    negated = True
                    j += 1
                chars = set()
                # Handle ] as first char in class
                first = True
                while j < len(pat) and pat[j] != ']':
                    if pat[j] == '\\' and j + 1 < len(pat):
                        chars.add(pat[j + 1])
                        j += 2
                    elif j + 2 < len(pat) and pat[j + 1] == '-' and pat[j + 2] != ']':
                        start = pat[j]
                        end = pat[j + 2]
                        for code in range(ord(start), ord(end) + 1):
                            chars.add(chr(code))
                        j += 3
                    else:
                        chars.add(pat[j])
                        j += 1
                    first = False
                if j < len(pat):
                    tokens.append(('class', chars, negated))
                    i = j + 1
                else:
                    # no closing bracket, treat '[' as literal
                    tokens.append(('literal', '['))
                    i += 1
            else:
                tokens.append(('literal', c))
                i += 1
        return tokens

    def split_path(path):
        if path == '':
            return []
        return path.split('/')

    pat_tokens = parse_pattern(pattern)
    path_segments = split_path(path)

    def dp(pi, si):
        if pi == len(pat_tokens):
            return si == len(path_segments)

        token = pat_tokens[pi]

        if token[0] == 'starstar':
            # Try matching zero or more whole segments
            # After '**', there might be more tokens
            # Try consuming 0, 1, 2, ... segments
            for k in range(si, len(path_segments) + 1):
                if dp(pi + 1, k):
                    return True
            return False

        if token[0] == 'star':
            # Match zero or more chars within a single segment
            # We need to figure out which segment we're in
            # Stars don't cross '/' boundaries, so they operate within a segment
            # But our tokens are flat, so we need to track segment position
            # Actually, let's restructure: we need to know current segment and position within it
            pass

        # For non-starstar tokens, we need segment-level matching
        # Let's use a different approach: match against the full path string but respect '/'
        pass

    # Better approach: work with the path as a string but track segment boundaries
    # Actually, let's do segment-by-segment matching with character-level within segments

    def match_segment(seg_pattern_tokens, seg):
        """Match a sequence of pattern tokens against a single path segment."""
        pi = 0
        si = 0
        memo = {}

        def dp(pi, si):
            key = (pi, si)
            if key in memo:
                return memo[key]
            if pi == len(seg_pattern_tokens):
                result = si == len(seg)
                memo[key] = result
                return result

            token = seg_pattern_tokens[pi]

            if token[0] == 'star':
                # Match zero or more chars (not '/')
                for k in range(si, len(seg) + 1):
                    if dp(pi + 1, k):
                        memo[key] = True
                        return True
                memo[key] = False
                return False

            if token[0] == 'question':
                if si < len(seg) and seg[si] != '/':
                    result = dp(pi + 1, si + 1)
                    memo[key] = result
                    return result
                memo[key] = False
                return False

            if token[0] == 'class':
                chars, negated = token[1], token[2]
                if si < len(seg):
                    c = seg[si]
                    if c == '/':
                        memo[key] = False
                        return False
                    in_class = c in chars
                    if negated:
                        in_class = not in_class
                    if in_class:
                        result = dp(pi + 1, si + 1)
                        memo[key] = result
                        return result
                memo[key] = False
                return False

            if token[0] == 'literal':
                if si < len(seg) and seg[si] == token[1]:
                    result = dp(pi + 1, si + 1)
                    memo[key] = result
                    return result
                memo[key] = False
                return False

            memo[key] = False
            return False

        return dp(0, 0)

    # Now match segments with '**' support
    # Flatten pattern into list of (segment_pattern_tokens) where '**' is a special marker
    # Actually, let's rebuild: split pattern by '/' but handle '**' specially

    # Let's re-parse into segments
    # A pattern like 'a/**/b' has segments: ['a', '**', 'b']
    # A pattern like 'a/*' has segments: ['a', '*'] where '*' is within the segment

    def parse_segments(pat):
        segments = []
        i = 0
        current = []
        while i < len(pat):
            if pat[i] == '/' and (not current or current[-1][0] != 'starstar'):
                # End of segment
                if current:
                    segments.append(current)
                    current = []
                i += 1
            else:
                # Parse a token
                if pat[i] == '\\':
                    if i + 1 < len(pat):
                        current.append(('literal', pat[i + 1]))
                        i += 2
                    else:
                        current.append(('literal', '\\'))
                        i += 1
                elif pat[i] == '?':
                    current.append(('question',))
                    i += 1
                elif pat[i] == '*':
                    if i + 1 < len(pat) and pat[i + 1] == '*':
                        current.append(('starstar',))
                        i += 2
                        while i < len(pat) and pat[i] == '*':
                            i += 1
                    else:
                        current.append(('star',))
                        i += 1
                elif pat[i] == '[':
                    j = i + 1
                    negated = False
                    if j < len(pat) and pat[j] == '!':
                        negated = True
                        j += 1
                    chars = set()
                    while j < len(pat) and pat[j] != ']':
                        if pat[j] == '\\' and j + 1 < len(pat):
                            chars.add(pat[j + 1])
                            j += 2
                        elif j + 2 < len(pat) and pat[j + 1] == '-' and pat[j + 2] != ']':
                            start = pat[j]
                            end = pat[j + 2]
                            for code in range(ord(start), ord(end) + 1):
                                chars.add(chr(code))
                            j += 3
                        else:
                            chars.add(pat[j])
                            j += 1
                    if j < len(pat):
                        current