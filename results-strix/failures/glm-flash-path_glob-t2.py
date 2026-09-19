def match(pattern, path):
    # Preprocess the pattern to handle escaped characters and '**'
    tokens = []
    i = 0
    n = len(pattern)
    while i < n:
        c = pattern[i]
        if c == '\\':
            if i + 1 < n:
                tokens.append(('LIT', pattern[i + 1]))
                i += 2
            else:
                tokens.append(('LIT', '\\'))
                i += 1
        elif c == '[':
            # Determine if it's a negated class [!...] or a standard class [...]
            is_negated = False
            if i + 1 < n and pattern[i + 1] == '!':
                is_negated = True
                i += 2
            
            # Parse the character class contents
            # We look for the closing ']', but we must handle escaped ']'
            class_start = i
            while i < n:
                if i + 1 < n and pattern[i] == '\\' and pattern[i + 1] == ']':
                    i += 2
                elif pattern[i] == ']':
                    break
                else:
                    i += 1
            
            if i >= n:
                # Unmatched '[' - treat as literal
                tokens.append(('LIT', '['))
                i = class_start
            else:
                # Extract class content
                class_content = pattern[class_start + 1 : i]
                tokens.append(('CHR_CLASS', class_content, is_negated))
                i += 1 # skip ']'
        elif c == '?':
            tokens.append(('ANY',))
            i += 1
        elif c == '*':
            if i + 1 < n and pattern[i + 1] == '*':
                tokens.append(('STAR_STAR',))
                i += 2
            else:
                tokens.append(('STAR',))
                i += 1
        else:
            tokens.append(('LIT', c))
            i += 1

    # Tokenize the path into segments
    segments = path.split('/') if path != '' else ['']
    
    # Memoization for backtracking
    memo = {}

    def recurse(p_idx, s_idx):
        state = (p_idx, s_idx)
        if state in memo:
            return memo[state]
        
        # Base Case: No more pattern tokens and no more path segments
        # (If we are at the end of path but there are segments left, fail unless they are empty)
        if p_idx == len(tokens):
            return s_idx == len(segments)
        
        # Handle remaining path segments if pattern ended (e.g. pattern 'a', path 'a/b')
        # The '*' at the end of the pattern handles this, but if there are no '*' tokens, 
        # the path must be exhausted.
        if s_idx == len(segments):
            # Check if remaining tokens are all STAR_STAR or STAR (which can match empty)
            # or if the path was empty and pattern is empty.
            for j in range(p_idx, len(tokens)):
                if tokens[j][0] not in ('STAR', 'STAR_STAR'):
                    return False
            return True

        token_type, *token_args = tokens[p_idx]
        seg = segments[s_idx]

        res = False
        if token_type == 'LIT':
            if seg == token_args[0]:
                res = recurse(p_idx + 1, s_idx + 1)
        elif token_type == 'ANY':
            if seg != '':
                res = recurse(p_idx + 1, s_idx + 1)
        elif token_type == 'STAR':
            # Matches empty or one segment
            if recurse(p_idx, s_idx + 1):
                res = True
            elif seg != '' and recurse(p_idx + 1, s_idx + 1):
                res = True
        elif token_type == 'STAR_STAR':
            # Matches zero or more segments
            # Option 1: Match zero segments (stay on same pattern token)
            if recurse(p_idx, s_idx + 1):
                res = True
            # Option 2: Match one segment and stay on STAR_STAR
            elif seg != '' and recurse(p_idx, s_idx + 1):
                res = True
            # Option 3: Match one segment and consume STAR_STAR (move to next pattern token)
            # This allows matching multiple segments in a row.
            elif seg != '' and recurse(p_idx + 1, s_idx + 1):
                res = True
        elif token_type == 'CHR_CLASS':
            class_content, is_negated = token_args
            # Handle ranges like a-z
            # Simple implementation: iterate chars. 
            # To support ranges, we'd need a parser, but usually these puzzles expect 
            # simple char sets or explicit ranges. Given the complexity of 
            # "[!a-z]" vs "[!a-z]", let's stick to char by char check or regex.
            # A manual parser for ranges is safer.
            char_match = False
            k = 0
            m = len(class_content)
            while k < m:
                c1 = class_content[k]
                if k + 2 < m and class_content[k+1] == '-':
                    c2 = class_content[k+2]
                    # Check if c1-c2 is a range (ASCII)
                    if ord(c1) <= ord(c2):
                        if ord(seg) >= ord(c1) and ord(seg) <= ord(c2):
                            char_match = True
                            break
                    k += 3
                else:
                    if c1 == seg:
                        char_match = True
                        break
                    k += 1
            
            if is_negated:
                res = not char_match
            else:
                res = char_match
            
            if res:
                res = recurse(p_idx + 1, s_idx + 1)

        memo[state] = res
        return res

    return recurse(0, 0)