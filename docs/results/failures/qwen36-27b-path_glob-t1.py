def match(pattern, path):
    def compile_pattern(pat):
        tokens = []
        i = 0
        n = len(pat)
        while i < n:
            c = pat[i]
            if c == '\\':
                i += 1
                if i < n:
                    tokens.append(('lit', pat[i]))
                else:
                    tokens.append(('lit', '\\'))
                i += 1
            elif c == '?':
                tokens.append(('any',))
                i += 1
            elif c == '*':
                # Check for **
                if i + 1 < n and pat[i + 1] == '*':
                    i += 2
                    # Check that ** is surrounded by / or boundaries
                    # We handle boundary logic in matching or by normalizing tokens
                    # Actually, let's just record it as a special token and handle boundaries in matching
                    # But standard glob semantics: ** must be a whole segment.
                    # Let's simplify: consume ** and the surrounding slashes if present?
                    # No, let's keep it simple. We'll parse segments.
                    # Actually, the prompt says "A path segment that is exactly '**'".
                    # So we should split by / first?
                    # But escaping makes splitting hard.
                    # Let's stick to character-by-character parsing but handle ** as a special token.
                    tokens.append(('starstar',))
                else:
                    tokens.append(('star',))
                i += 1
            elif c == '[':
                # Character class
                i += 1
                neg = False
                chars = set()
                if i < n and pat[i] == '!':
                    neg = True
                    i += 1
                elif i < n and pat[i] == '^':
                    neg = True
                    i += 1
                
                # If the first char is ']', it's a literal ']'
                if i < n and pat[i] == ']':
                    chars.add(']')
                    i += 1
                
                while i < n and pat[i] != ']':
                    if i + 2 < n and pat[i + 1] == '-' and pat[i + 2] != ']':
                        start = pat[i]
                        end = pat[i + 2]
                        for code in range(ord(start), ord(end) + 1):
                            chars.add(chr(code))
                        i += 3
                    else:
                        chars.add(pat[i])
                        i += 1
                
                if i < n:
                    i += 1 # consume ']'
                else:
                    # Unterminated bracket, treat as literal? Or error?
                    # Usually treat as literal. Let's push back.
                    i -= 1
                    chars = set()
                    neg = False
                    # Re-parse the '[' as literal?
                    # Let's just assume well-formed for simplicity or treat as literal '['
                    # Reset i to before '['
                    # This is tricky. Let's assume valid input or just fail.
                    # For robustness, let's just treat the whole thing as literal if it fails.
                    # But we already consumed chars.
                    # Let's just not add a class token if it's invalid.
                    pass 
                
                if i <= n: # If we consumed a ']', we are good
                    # Check if we actually parsed a class
                    # If i didn't advance past the start, it's invalid
                    pass
                tokens.append(('class', chars, neg))
            elif c == '/':
                tokens.append(('slash',))
                i += 1
            else:
                tokens.append(('lit', c))
                i += 1
        return tokens

    def matches(tokens, t_idx, path, p_idx):
        # t_idx: index in tokens
        # p_idx: index in path string
        
        while t_idx < len(tokens):
            token = tokens[t_idx]
            
            # If we have no more path characters left
            if p_idx == len(path):
                # Remaining tokens must be empty or just stars that can match empty
                # But we need to match the whole path.
                # If there are remaining tokens, they must be able to match empty string.
                # Only 'star' can match empty. 'starstar' can match empty (zero segments).
                # But 'star' cannot cross '/'. 'starstar' can.
                # Let's check if remaining tokens can match empty.
                remaining = tokens[t_idx:]
                if all(t == ('star',) or t == ('starstar',) for t in remaining):
                    return True
                return False
            
            tok_type = token[0]
            
            if tok_type == 'slash':
                if path[p_idx] != '/':
                    return False
                p_idx += 1
                t_idx += 1
                
            elif tok_type == 'lit':
                if path[p_idx] != token[1]:
                    return False
                p_idx += 1
                t_idx += 1
                
            elif tok_type == 'any':
                # Matches exactly one char, not /
                if path[p_idx] == '/':
                    return False
                p_idx += 1
                t_idx += 1
                
            elif tok_type == 'star':
                # Matches zero or more chars, not /
                # Try matching 0, 1, 2, ... chars until '/' or end
                # We need to find the next '/' or end of string
                # And try matching the rest
                
                # Find the next slash or end
                next_slash = -1
                for k in range(p_idx, len(path)):
                    if path[k] == '/':
                        next_slash = k
                        break
                
                # Try matching from p_idx to next_slash (exclusive) or end
                # The range of characters * can match is from p_idx to next_slash (if exists) or len(path)
                limit = next_slash if next_slash != -1 else len(path)
                
                # Try each possible end position for *
                # * matches path[p_idx:end_pos]
                # Then we try to match remaining tokens with path[end_pos:]
                
                for end_pos in range(p_idx, limit + 1):
                    if matches(tokens, t_idx + 1, path, end_pos):
                        return True
                return False
                
            elif tok_type == 'starstar':
                # Matches zero or more whole segments.
                # This means it matches:
                # 1. Empty (zero segments)
                # 2. One or more segments, where each segment is separated by /
                #    So it matches: "" or "/seg1" or "/seg1/seg2" etc.
                #    But wait, the pattern is 'a/**/b'.
                #    The ** is between slashes.
                #    So ** matches:
                #       - empty (so a//b -> a/b? No, usually ** consumes the surrounding slashes in some implementations, 
                #         but here our tokens are explicit.
                #         If pattern is a/**/b, tokens are: lit a, slash, starstar, slash, lit b.
                #         path a/b: lit a matches a, slash matches /, starstar matches empty, slash matches /? No, next char is b.
                #         So starstar must be able to match the intermediate segments AND the slashes.
                
                # Let's redefine: ** matches zero or more segments.
                # A segment is a sequence of non-slash chars.
                # So ** matches:
                #   - empty string (0 segments)
                #   - /seg1 (1 segment, with leading slash)
                #   - /seg1/seg2 (2 segments)
                #   etc.
                # But this depends on how it's surrounded.
                
                # Standard fnmatch/glob behavior for **:
                # If ** is a whole segment, it matches any number of segments.
                # In our token stream, ** is a token.
                # If we have ... / ** / ...
                # The ** token should match:
                #   - empty string (if the surrounding slashes are consumed by the neighbors)
                #   - /seg1
                #   - /seg1/seg2
                #   etc.
                
                # Actually, it's easier to think of ** as matching:
                #   - ""
                #   - "/<anything not containing //>"
                #   - "/<seg1>/<anything>"
                
                # Let's try a different approach:
                # ** matches zero or more segments.
                # A segment is matched by [^/]*.
                # So ** matches ( / [^/]* )*
                
                # We can try matching:
                # 1. Empty string (0 segments)
                # 2. / followed by a segment (matched by *) followed by recursively matching **
                
                # Try empty match
                if matches(tokens, t_idx + 1, path, p_idx):
                    return True
                
                # Try matching one or more segments
                # Each segment starts with /
                if p_idx < len(path) and path[p_idx] == '/':
                    # Match the slash
                    # Then match a segment (non-slash chars)
                    # Then recursively match **
                    
                    # Find end of current segment
                    seg_start = p_idx + 1
                    seg_end = seg_start
                    while seg_end < len(path) and path[seg_end] != '/':
                        seg_end += 1
                    
                    # The segment is path[seg_start:seg_end]
                    # We need to match this segment with something?
                    # No, ** just consumes it.
                    # So we consume / and the segment, then try ** again.
                    
                    # But we need to make sure the segment is not empty?
                    # Usually segments are non-empty.
                    if seg_start < seg_end:
                        if matches(tokens, t_idx, path, seg_end):
                            return True
                    else:
                        # Empty segment? e.g. //
                        # Usually invalid or treated as empty.
                        # Let's just consume the slash and try again.
                        if matches(tokens, t_idx, path, p_idx + 1):
                            return True
                            
                return False
                
            elif tok_type == 'class':
                chars = token[1]
                neg = token[2]
                c = path[p_idx]
                if c == '/':
                    return False
                if neg:
                    if c in chars:
                        return False
                else:
                    if c not in chars:
                        return False
                p_idx += 1
                t_idx += 1
            else:
                return False
                
        # All tokens consumed
        return p_idx == len(path)
    
    tokens = compile_pattern(pattern)
    return matches(tokens, 0, path, 0)