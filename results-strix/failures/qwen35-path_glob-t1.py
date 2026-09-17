```python
def match(pattern, path):
    def parse_pattern(p):
        tokens = []
        i = 0
        while i < len(p):
            c = p[i]
            if c == '\\':
                if i + 1 < len(p):
                    tokens.append(('char', p[i+1]))
                    i += 2
                else:
                    tokens.append(('char', '\\'))
                    i += 1
            elif c == '*':
                if i + 1 < len(p) and p[i+1] == '*':
                    if i + 2 < len(p) and p[i+2] == '/':
                        tokens.append(('starstar',))
                        i += 3
                    elif i == 0 or (i > 0 and p[i-1] == '/'):
                        # Handle leading ** or isolated ** not followed by /
                        # Actually, standard behavior is ** matches across slashes
                        # If we see ** followed by /, it's a multi-segment match.
                        # If we see ** at end or not followed by /, treat as multi-segment too.
                        # Let's simplify: ** always matches zero or more segments.
                        # We need to check if it's followed by a slash to know if it consumes segments.
                        # Actually, standard glob: ** matches any number of directories including zero.
                        # It effectively means "match any sequence of / and chars".
                        # But the spec says "zero or more whole segments".
                        tokens.append(('starstar',))
                        i += 2
                        # Skip the slash if present to avoid double processing
                        if i < len(p) and p[i] == '/':
                            i += 1
                    else:
                        tokens.append(('star',))
                        i += 1
                else:
                    tokens.append(('star',))
                    i += 1
            elif c == '?':
                tokens.append(('question',))
                i += 1
            elif c == '[':
                j = i + 1
                neg = False
                if j < len(p) and p[j] == '!':
                    neg = True
                    j += 1
                # Find closing bracket
                while j < len(p) and p[j] != ']':
                    j += 1
                if j < len(p):
                    chars = p[i+1:j]
                    if neg:
                        tokens.append(('class', chars, True))
                    else:
                        tokens.append(('class', chars, False))
                    i = j + 1
                else:
                    tokens.append(('char', '['))
                    i += 1
            else:
                tokens.append(('char', c))
                i += 1
        return tokens

    def tokenize_path(p):
        segments = p.split('/')
        return segments

    def match_segment(segment, token_seq, seg_idx, token_idx):
        # Helper for recursive matching logic
        # We will implement a pointer-based matching instead of pre-tokenizing path into chars
        # to handle the ** logic correctly.
        pass

    # Re-implementing logic with two pointers for pattern and path segments
    # to handle ** correctly.
    
    pat_tokens = parse_pattern(pattern)
    path_segments = path.split('/') if path else []
    
    # If pattern has no slashes but path is empty, handle edge cases?
    # If pattern is empty, only matches empty path.
    # If path is empty, only matches empty pattern or pattern that matches empty string.
    
    # We need a recursive function that handles segments.
    # state: pat_idx, seg_idx
    
    def match_recursive(pat_idx, seg_idx):
        # pat_idx: index in pat_tokens
        # seg_idx: index in path_segments
        
        while pat_idx < len(pat_tokens):
            token = pat_tokens[pat_idx]
            
            if token[0] == 'starstar':
                # Try to match zero segments first (if next token can match current seg)
                # Or match one segment and recurse
                # ** matches zero or more segments.
                # It acts as a bridge.
                
                # Option 1: Consume zero segments (if next token matches current segment)
                # Option 2: Consume one segment, then try to match rest with current **
                
                # Try to match remaining pattern starting from next token, skipping this **
                # But ** can match multiple segments.
                
                # Logic:
                # If we are at the end of segments, we must match remaining pattern with zero segments.
                # If we have segments, we can either:
                # 1. Match zero segments: try match(pat_idx+1, seg_idx)
                # 2. Match one segment: consume seg_idx, stay at pat_idx (since ** can match more), recurse
                
                # However, ** matches "whole segments".
                # So if we have "a/**/b" and path "a/x/b":
                # match a -> match a
                # match ** -> try zero segments? No, then we need to match b against x. Fail.
                # Then try match one segment (x), stay at **, then try to match b against b. Success.
                
                # Optimization:
                # If we are at the last token and it's starstar, it matches remaining segments (even if empty? Yes)
                # But we need to be careful.
                
                # Let's try greedy approach or backtracking.
                
                # Try matching zero segments first
                if match_recursive(pat_idx + 1, seg_idx):
                    return True
                
                # If we have segments left, try consuming one segment
                if seg_idx < len(path_segments):
                    # Consume the segment, keep pat_idx as starstar to allow more
                    if match_recursive(pat_idx, seg_idx + 1):
                        return True
                
                return False
            
            elif token[0] == 'star':
                # * matches zero or more characters in current segment, but not '/'
                # Since we split by '/', we are working on a single segment.
                # We need to match the rest of the segment against the rest of the pattern.
                # But the pattern might have multiple tokens.
                # This is tricky. Let's switch to character-level matching for the current segment.
                
                # We need to match the current segment (path_segments[seg_idx]) against the pattern starting at pat_idx
                # But the pattern might continue after *.
                
                # Let's create a helper to match a string segment against a list of tokens.
                # But we need to handle the case where * matches 0 chars.
                
                # If we consume *, we need to see if the rest of the pattern matches the rest of the string.
                # Or if * matches more chars, we need to try again.
                
                # This suggests we need a function match_string(segment, tokens)
                # But we are in a segment loop.
                
                # Let's restructure:
                # If token is *, it matches 0 or more chars in current segment.
                # We need to try all lengths from 0 to len(segment).
                # But the pattern after * might match the remaining chars.
                
                # Actually, standard glob: * matches any sequence of non-slash characters.
                # Since we split by /, we are already inside a segment.
                
                # We need to match segment[0...k] with *? No, * is in the pattern.
                # The pattern token * must match a substring of the current segment.
                # But the segment is fixed.
                # We need to find if there exists a split point in the segment such that:
                # prefix matches part of pattern before *, suffix matches part of pattern after *
                # But here * is a single token.
                # So * matches some prefix of the segment, and the rest of the pattern matches the rest.
                # OR * matches the whole segment, and the rest of the pattern matches empty (if any).
                
                # Wait, the pattern is linear.
                # If we have "a*b", and segment "axyz", * matches "xy".
                # We need to try all lengths for *.
                
                # Let's implement a helper for matching a string against a token list.
                return match_segment_string(path_segments[seg_idx], pat_tokens[pat_idx:])
            
            elif token[0] == 'class':
                # Matches exactly one character
                if seg_idx >= len(path_segments):
                    return False