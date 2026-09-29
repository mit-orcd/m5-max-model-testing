```python
def match(pattern, path):
    def compile_pattern(p):
        # Returns a list of tokens: ('LITERAL', char), ('QUESTION',), ('STAR',), ('DOUBLE_STAR',), ('CLASS', [chars])
        tokens = []
        i = 0
        n = len(p)
        while i < n:
            c = p[i]
            if c == '\\':
                if i + 1 < n:
                    tokens.append(('LITERAL', p[i+1]))
                    i += 2
                else:
                    tokens.append(('LITERAL', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('QUESTION',))
                i += 1
            elif c == '*':
                if i + 1 < n and p[i+1] == '*':
                    # Check if it's a double star segment (surrounded by / or start/end)
                    # We need to verify it's a whole segment match
                    # For now, collect consecutive stars, but the spec says exactly '**'
                    # Let's handle the segment logic during matching or pre-process segments
                    # Actually, the spec says "A path segment that is exactly '**'"
                    # So we need to split by '/' first to identify segments?
                    # Or handle it in the regex-like engine.
                    # Let's stick to tokenizing the pattern string, but mark double stars carefully.
                    # If we see ** at start, end, or between /, it's a double star.
                    # But simpler: tokenize normally, then in matching logic handle ** specially?
                    # No, the token stream needs to know.
                    # Let's assume the pattern is split into segments by '/' for the ** logic.
                    # Actually, let's just tokenize and handle ** as a special token when we see it.
                    # But ** can be part of a segment like 'a**b' which is not a double star segment.
                    # The spec: "A path segment that is exactly '**'".
                    # So we must split pattern by '/' first.
                    pass
                else:
                    tokens.append(('STAR',))
                    i += 1
            elif c == '[':
                # Parse character class
                j = i + 1
                neg = False
                if j < n and p[j] == '!':
                    neg = True
                    j += 1
                chars = set()
                if j < n and p[j] == ']':
                    # Empty class or class starting with ]
                    # Spec says '[!...]' for negation, implies at least one char?
                    # Usually '[]' is empty, '[a]' is 'a'.
                    # If '[]' it matches nothing? Or just ']'?
                    # Let's assume standard behavior: if ']' is first, it's included.
                    if neg:
                        # If negated and first char is ], then ] is not in set?
                        # Actually, standard glob: [!] means negation, then ] is literal.
                        # Let's implement standard: if first char after [! is ], it's literal.
                        pass
                    else:
                        chars.add(']')
                        j += 1
                else:
                    while j < n and p[j] != ']':
                        if j + 2 < n and p[j+1] == '-' and p[j+2] != ']':
                            # Range
                            start = p[j]
                            end = p[j+2]
                            for code in range(ord(start), ord(end) + 1):
                                chars.add(chr(code))
                            j += 3
                        else:
                            chars.add(p[j])
                            j += 1
                    if j < n:
                        j += 1 # skip ]
                
                if neg:
                    # We need to know the universe? No, just store negation and allowed chars.
                    # But matching logic needs to know what is NOT allowed.
                    # We can store (neg, chars)
                    tokens.append(('CLASS', chars, neg))
                else:
                    tokens.append(('CLASS', chars, False))
                i = j
            else:
                tokens.append(('LITERAL', c))
                i += 1
        return tokens

    # Re-implement tokenization to handle ** segments correctly
    # Split pattern by '/' to handle ** segments
    # But ** can be escaped? No, backslash escapes next char.
    # So we can't just split blindly.
    # Let's tokenize the whole string, but when we see ** we need to check context.
    # Actually, the easiest way is to split the pattern into segments by '/', but respecting escapes.
    
    def split_pattern(p):
        segments = []
        current = []
        i = 0
        n = len(p)
        while i < n:
            c = p[i]
            if c == '\\' and i + 1 < n:
                current.append(c)
                current.append(p[i+1])
                i += 2
            elif c == '/':
                segments.append(''.join(current))
                current = []
                i += 1
            else:
                current.append(c)
                i += 1
        segments.append(''.join(current))
        return segments

    pattern_segments = split_pattern(pattern)
    path_segments = path.split('/') if path else []

    # If path is empty string, path_segments is ['']?
    # If path is "", split gives ['']?
    # If path is "a", split gives ['a']
    # If path is "", we want to match pattern ""?
    # Let's handle empty path separately.
    if path == "":
        return pattern == ""
    
    # If path is not empty, split gives list of non-empty strings?
    # "a/b" -> ['a', 'b']
    # "a" -> ['a']
    # "" -> [''] (if we use split)
    # But if path is "", we handled it.
    # What if path is "/"? split gives ['', '']?
    # The problem says "Paths are separated by '/'". Usually paths don't start with / unless absolute.
    # Let's assume relative paths, no leading slash unless specified.
    # If path is "a//b", split gives ['a', '', 'b'].
    # We should probably treat consecutive slashes as one? Or empty segments?
    # Spec doesn't say. Let's assume standard split behavior.
    
    # Actually, let's re-evaluate split.
    # If path is "a/b", segments = ['a', 'b']
    # If path is "a", segments = ['a']
    # If path is "", segments = ['']?
    # Let's use: path_segments = path.split('/') if path else []
    # But if path is "a", split gives ['a'].
    # If path is "", split gives ['']? No, "".split('/') gives [''].
    # So if path is "", we have one segment which is empty.
    # But we handled path == "" above.
    # What if path is "/"? split gives ['', ''].
    # Let's assume paths don't have leading/trailing slashes unless they are part of the structure.
    # Let's just use split.
    
    # Now match segments
    # We need to handle ** which matches zero or more segments.
    
    def match_segments(p_segs, h_segs):
        # p_segs: list of pattern segments
        # h_segs: list of path segments
        # Returns True if full match
        
        # We need to handle ** in pattern segments.
        # A segment is "**" if it is exactly "**".
        # But what if it's "a**b"? That's not a double star segment.
        # So we check if segment == "**".
        
        # We can use recursion or iteration.
        # Since ** can match multiple segments, we need to try all possibilities.
        
        pi = 0
        hi = 0
        
        while pi < len(p_segs) and hi < len(h_segs):
            p_seg = p_segs[pi]
            
            if p_seg == '**':
                # Check if next segment is also **? No, spec says exactly '**'
                # But what if pattern is 'a/**/**/b'?
                # We treat each ** independently.