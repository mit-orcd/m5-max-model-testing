```python
def match(pattern, path):
    # Tokenize pattern into a list of regex-like components or special tokens
    # We will use a recursive descent parser approach with backtracking or dynamic programming
    # Since the rules are complex (especially ** and character classes), we'll implement a custom matcher.
    
    # Preprocess: handle escape sequences and tokenize
    # We'll convert the pattern into a list of atoms:
    # - literal char
    # - '?' -> single char (not /)
    # - '*' -> zero or more chars (not /)
    # - '**' -> zero or more segments
    # - class -> set of chars (not /)
    
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
        elif c == '?':
            tokens.append(('Q',))
            i += 1
        elif c == '*':
            if i + 1 < n and pattern[i+1] == '*':
                # Check if it's exactly '**' and not part of a longer sequence like '***'
                # But the spec says '**' matches whole segments. If we see '***', it's ambiguous.
                # Usually, '**' is treated as a special token. If there are more stars, we treat '**' as the token and the rest as '*' or literal?
                # The spec says: "A path segment that is exactly '**' matches zero or more whole segments"
                # So we look for exactly '**' as a segment. But in a pattern string, we might have 'a/**/b' or 'a/**b'.
                # We need to tokenize greedily for '**' when it appears.
                # If we have '***', we can treat the first two as '**' and the third as '*'? Or is '***' invalid?
                # Standard glob: '***' is equivalent to '**' + '*' or just '*' depending on implementation.
                # Let's assume: if we see '**', we consume two stars. If followed by another star, we treat the first two as '**' and the next as '*' (which matches chars).
                # However, the problem says "A path segment that is exactly '**'". This implies we are matching segments.
                # Let's tokenize: if we see '**', we take it as a special token. If we see a single '*', we take it.
                # What about '***'? We'll take '**' then '*'.
                tokens.append(('STAR2',))
                i += 2
                # If there is another star, it will be processed in next iteration as '*'?
                # But we need to be careful. If we have '***', i is now at index of 3rd star.
                # Next loop will see '*', append ('STAR',). Correct.
            else:
                tokens.append(('STAR',))
                i += 1
        elif c == '[':
            # Parse character class
            j = i + 1
            if j < n and pattern[j] == '!':
                negated = True
                j += 1
            else:
                negated = False
                
            if j < n and pattern[j] == ']':
                j += 1  # Allow ']' as first char
                
            chars = set()
            while j < n and pattern[j] != ']':
                if pattern[j] == '\\' and j + 1 < n:
                    chars.add(pattern[j+1])
                    j += 2
                elif j + 2 < n and pattern[j+1] == '-' and pattern[j+2] != ']':
                    # Range
                    start = pattern[j]
                    end = pattern[j+2]
                    for code in range(ord(start), ord(end) + 1):
                        chars.add(chr(code))
                    j += 3
                else:
                    chars.add(pattern[j])
                    j += 1
            
            if j < n and pattern[j] == ']':
                j += 1
            else:
                # Malformed, treat as literal? Or invalid? We'll assume well-formed per spec.
                # If no closing bracket, we might treat the rest as literal? But spec implies valid patterns.
                # We'll just break and treat the rest as literal? Or raise? Let's assume valid input.
                pass
            
            tokens.append(('CLASS', chars, negated))
            i = j
        else:
            tokens.append(('LIT', c))
            i += 1
            
    path_segments = path.split('/') if path else []
    
    # Now we have tokens and path_segments.
    # We need to match tokens against path_segments.
    # '*' and '?' match within a segment.
    # '**' matches zero or more segments.
    # We'll use recursion with memoization.
    
    memo = {}
    
    def dp(t_idx, p_idx):
        state = (t_idx, p_idx)
        if state in memo:
            return memo[state]
        
        # If we consumed all tokens
        if t_idx == len(tokens):
            return p_idx == len(path_segments)
        
        # If we consumed all path segments but have tokens left
        if p_idx == len(path_segments):
            # Can we consume remaining tokens? Only if they are '**' or empty matches?
            # Actually, if p_idx is end, we can only match if remaining tokens are all '**' that can match zero segments?
            # But '**' matches zero or more segments. If we have multiple '**', they all match zero.
            # However, we might have '*' or '?' or literals that need to match a segment.
            # So if p_idx is end, we can only succeed if the remaining tokens can match zero segments.
            # Only '**' can match zero segments. '*' and '?' and literals need at least one segment (or part of one).
            # Wait: '*' matches zero or more characters in a segment. But if there is no segment, can '*' match?
            # The path is split into segments. If p_idx == len, we have no segments left.
            # A '*' in a segment requires a segment to exist. So if we are at the end of segments, we can't match '*' unless it's part of a '**' logic?
            # Actually, our tokenization splits by segments implicitly? No, we split the path into segments.
            # The tokens are matched against segments.
            # So if we are at the end of segments, we can only match if the remaining tokens are all '**' (which can match zero segments) or if there are no tokens.
            # But what if we have a token '*'? It must match a segment. So if p_idx == len, we fail for '*'.
            # So: if p_idx == len, we check if all remaining tokens are '**'.
            for k in range(t_idx, len(tokens)):
                if tokens[k][0] != 'STAR2':
                    memo[state] = False
                    return False
            memo[state] = True
            return True
        
        token = tokens[t_idx]
        t_type = token[0]
        
        if t_type == 'LIT':
            # Must match exactly one segment
            seg = path_segments[p_idx]
            if seg == token[1]:
                res = dp(t_idx + 1, p_idx + 1)
            else:
                res = False
            memo[state] = res
            return res
            
        elif t_type == 'Q':
            # Matches exactly one char in a segment, not '/'
            seg = path_segments[p_idx]
            if len(seg) == 1 and seg != '/':
                res = dp(t_idx + 1, p_idx + 1)
            else:
                res = False
            memo[state] = res
            return res
            
        elif t_type == 'STAR':
            # Matches zero or more chars in a segment (not '/')
            # But we are matching against a whole segment?
            # Actually, '*' is inside a segment. But our tokenization doesn't split segments.
            # We need to match the token against the current segment.
            # However, the structure of the matcher is: tokens are matched against segments.
            # But '*' and '?' and classes and literals can be mixed within a segment.
            # Our current tokenization approach is flawed because we treat the whole pattern as a sequence of tokens that map to segments.
            # But a segment can contain multiple tokens.
            # We need to restructure: the pattern is a sequence of tokens, and we match them against the sequence of segments.
            # But within a segment, we can have a sequence of tokens.
            # So we need to handle the case where multiple tokens belong to the same segment.
            # This is getting complicated.
            
            # Alternative approach:
            # We are matching a pattern (list of tokens) against a path (list of segments).
            # The pattern can be split into "segments" of tokens, where a segment of tokens corresponds to one path segment.
            # The special token '**' separates these groups.
            # So:
            # Pattern: [group1] ['**'] [group2] ...
            # Group1 must match path[0], group2 must match path[1], etc.
            # But '**' can match zero or more segments, so it can skip groups.
            
            # Let's change the strategy:
            # We'll split the tokens into groups separated by 'STAR2'.
            # Each group is a list of tokens that must match a single path segment.
            # Then we match the groups against the path segments, with the ability to skip groups for '**'.
            
            # But wait, the original problem says:
            # '*' matches zero or more characters but never '/'.
            # So '*' is within a segment.
            # '**' matches zero or more whole segments.
            # So we need to parse the pattern into segments of tokens.
            
            # Let's re-tokenize the pattern into a list of "segment patterns".
            # A segment pattern is a list of tokens that match one path segment.
            # The entire pattern is a list of these segment patterns, separated by 'STAR2' which acts as a wildcard for segments.
            
            # Actually, we can do:
            # tokens = ... (as before)
            # Now, we want to match the tokens against the path segments.
            # We can use a recursive function that tries to match the current token against the current segment, and if it succeeds, move to next token/segment.
            # But if the token is 'STAR', it can match part of the segment, and then we continue with the next token in the same segment.
            # This is too complex for a single dp state.
            
            # Let's try a different approach:
            # Convert the pattern into a regex-like structure but for segments.
            # Or, use a state machine that tracks:
            # - current token index
            # - current path segment index
            # - position within the current segment (if we are matching a segment pattern)
            
            # Given the complexity, let's try to implement a matcher that works on the token list and segment list,
            # but handles the intra-segment matching for '*', '?', etc.
            
            # We'll define a function match_segment(tokens, seg) that returns True if the tokens match the segment.
            # But tokens might span multiple segments if there is no '**'? No, tokens for a segment are contiguous until '**' or end.
            # Actually, the pattern is a sequence of tokens. The '**' token is a separator that allows skipping segments.
            # So we can split the tokens into blocks separated by 'STAR2'.
            # Each block must match one segment.
            # Then we match the blocks against the segments, with the ability to skip segments for the '**' tokens.
            
            # Let's restructure the token list into a list of (block, is_star2) where block is a list of tokens for a segment, and is_star2 is True if there was a '**' before it.
            # Actually, we can have:
            # pattern: [block1] ['**'] [block2] ['**'] [block3]
            # This matches: block1 -> seg0, block2 -> seg1, block3 -> seg2
            # Or: block1 -> seg0, block2 -> seg2 (skipping seg1), etc.
            # But also: ['**'] at the beginning means we can skip seg0, seg1, etc.
            
            # Steps:
            # 1. Split tokens by 'STAR2' into blocks.
            # 2. Each block is a list of tokens that must match a single segment.
            # 3. The pattern of blocks must match the path segments, with the ability to skip segments between blocks (because of the '**' that separated them).
            
            # However, the '**' token is not just a separator; it can also be at the beginning or end.
            # Example: 'a/**/b' -> blocks: [ [LIT('a')] , [LIT('b')] ] with two '**' around the middle block?
            # Actually, 'a/**/b' tokens: [LIT('a'), STAR2, LIT('b')]
            # We split by STAR2: blocks = [ [LIT('a')], [LIT('b')] ]
            # And we have two STAR2s? No, we split by STAR2, so we get the parts between them.
            # The number of blocks is (number of STAR2) + 1.
            # But the STAR2 tokens are the separators.
            
            # Let's do:
            # blocks = []
            # current_block = []
            # for token in tokens:
            #   if token is STAR2:
            #       blocks.append(current_block)
            #       current_block = []
            #       # Also, we need to remember that there is a STAR2 here
            #   else:
            #       current_block.append(token)
            # blocks.append(current_block)
            #
            # Now, we have a list of blocks. The pattern matches if we can assign each block to a distinct segment in the path, in order, with the condition that between two blocks, we can skip any number of segments (because of the STAR2).
            # But also, the first block can be after some skipped segments (if the pattern starts with STAR2), and the last block can be before some skipped segments (if the pattern ends with STAR2).
            # Actually, the STAR2 tokens are between the blocks.
            # So if we have blocks [B0, B1, B2], then the pattern is B0 ** B1 ** B2.
            # This means:
            #   B0 matches some segment i
            #   B1 matches some segment j (j > i)
            #   B2 matches some segment k (k > j)
            # And we can have zero or more segments between them.
            # Also, if the pattern starts with STAR2, then B0 can match segment 0, 1, 2, ...
            # If the pattern ends with STAR2, then the last block can match any segment up to the end.
            # But our splitting doesn't capture the leading/trailing STAR2.
            # We need to know if the pattern starts with STAR2 and ends with STAR2.
            
            # Let's do:
            # blocks = []
            # current_block = []
            # leading_star2 = False
            # trailing_star2 = False
            # if tokens and tokens[0] == ('STAR2',):
            #     leading_star2 = True
            #     tokens = tokens[1:]
            # if tokens and tokens[-1] == ('STAR2',):
            #     trailing_star2 = True
            #     tokens = tokens[:-1]
            #
            # Then split the remaining tokens by STAR2.
            # But this is messy.
            
            # Alternative: use a recursive function that handles the matching of a block of tokens against a segment, and then recursively matches the rest of the blocks against the rest of the segments, with the ability to skip segments for STAR2.
            
            # Given the time, let's implement a simpler version that works for the common cases.
            # We'll use a recursive function that tries to match the current block of tokens against the current segment, and then moves to the next block and the next segment, or skips segments for STAR2.
            
            # But we need to handle the case where a block has multiple tokens that must match a single segment.
            # So we need a function match_block(tokens, segment) that returns True if the tokens match the segment.
            # Then we can use dp(block_idx, seg_idx) to match the rest.
            
            # Let's define match_block(tokens, segment):
            #   This is a subproblem: match a list of tokens against a single string (segment).
            #   We can use a simple recursive function with memoization for this.
            #   The tokens in the block are: LIT, Q, STAR, CLASS.
            #   STAR in a block means zero or more characters (not '/').
            #   So we can match the block against the segment using a standard glob matching for a single string.
            
            # Steps for the whole solution:
            # 1. Tokenize the pattern into tokens (as before).
            # 2. Split the tokens into