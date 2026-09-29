def match(pattern, path):
    def parse_pattern(p):
        tokens = []
        i = 0
        while i < len(p):
            c = p[i]
            if c == '\\':
                if i + 1 < len(p):
                    tokens.append(('literal', p[i + 1]))
                    i += 2
                else:
                    tokens.append(('literal', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('question',))
                i += 1
            elif c == '*':
                # Collect consecutive stars
                count = 0
                while i < len(p) and p[i] == '*':
                    count += 1
                    i += 1
                if count >= 2:
                    # Check if it's exactly '**' (or more, but treat as doublestar)
                    # Actually, '**' means zero or more segments
                    # More than two stars: treat as doublestar for simplicity? 
                    # Spec says "A path segment that is exactly '**'"
                    # So we need to check if the stars form a segment boundary
                    pass
                # We'll handle this differently - collect the star run
                # and check if it's exactly two stars as a segment
                # Let's redo: collect star run, then check context
                tokens.append(('star', count))
            elif c == '[':
                # Parse character class
                j = i + 1
                negate = False
                if j < len(p) and p[j] == '!':
                    negate = True
                    j += 1
                chars = set()
                # Handle ] as first char in class
                while j < len(p) and p[j] != ']':
                    if j + 2 < len(p) and p[j + 1] == '-' and p[j + 2] != ']':
                        # Range
                        start = p[j]
                        end = p[j + 2]
                        for code in range(ord(start), ord(end) + 1):
                            chars.add(chr(code))
                        j += 3
                    else:
                        chars.add(p[j])
                        j += 1
                if j < len(p):
                    j += 1  # skip ]
                tokens.append(('class', chars, negate))
                i = j
            else:
                tokens.append(('literal', c))
                i += 1
        return tokens

    def tokenize_path(path):
        return path.split('/')

    pat_tokens = parse_pattern(pattern)
    path_parts = tokenize_path(path)

    def match_tokens(ptokens, pp, parts, pi):
        # ptokens: pattern tokens, pp: current pattern index
        # parts: path parts (already split), pi: current part index
        # But '**' matches across parts, so we need to work with the raw path or handle segment matching
        
        # Actually, let's work with the path as a string and pattern as tokens,
        # but handle ** specially by matching segments.
        pass

    # Let's use a recursive approach working on the path string
    def recurse(pat_idx, path_idx):
        p_len = len(pat_tokens)
        h_len = len(path)
        
        while pat_idx < p_len and path_idx < h_len:
            token = pat_tokens[pat_idx]
            
            if token[0] == 'literal':
                if path[path_idx] != token[1]:
                    return False
                pat_idx += 1
                path_idx += 1
            elif token[0] == 'question':
                if path[path_idx] == '/':
                    return False
                pat_idx += 1
                path_idx += 1
            elif token[0] == 'class':
                chars, negate = token[1], token[2]
                c = path[path_idx]
                if c == '/':
                    return False
                if negate:
                    if c in chars:
                        return False
                else:
                    if c not in chars:
                        return False
                pat_idx += 1
                path_idx += 1
            elif token[0] == 'star':
                star_count = token[1]
                if star_count >= 2:
                    # ** matches zero or more whole segments
                    # Try matching zero segments, one segment, two segments, etc.
                    for skip in range(0, h_len - path_idx + 1):
                        # Check if skipping 'skip' chars ends at a segment boundary or end
                        if skip == 0 or path[path_idx + skip - 1] == '/':
                            if recurse(pat_idx + 1, path_idx + skip):
                                return True
                    return False
                else:
                    # * matches zero or more chars but not /
                    while path_idx < h_len and path[path_idx] != '/':
                        if recurse(pat_idx + 1, path_idx):
                            return True
                        path_idx += 1
                    # Also try matching zero chars
                    if recurse(pat_idx + 1, path_idx):
                        return True
                    return False
        
        # Consume remaining ** in pattern
        while pat_idx < p_len and pat_tokens[pat_idx][0] == 'star' and pat_tokens[pat_idx][1] >= 2:
            pat_idx += 1
        
        return pat_idx == p_len and path_idx == h_len

    return recurse(0, 0)