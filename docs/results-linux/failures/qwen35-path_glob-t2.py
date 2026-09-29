def match(pattern, path):
    def parse_pattern(p):
        tokens = []
        i = 0
        while i < len(p):
            c = p[i]
            if c == '*':
                if i + 1 < len(p) and p[i + 1] == '*':
                    if i + 2 < len(p) and p[i + 2] == '/':
                        tokens.append(('STAR_STAR',))
                        i += 3
                        continue
                    elif i + 2 == len(p):
                        tokens.append(('STAR_STAR',))
                        i += 3
                        continue
                tokens.append(('STAR',))
            elif c == '?':
                tokens.append(('Q',))
            elif c == '[':
                j = i + 1
                if j < len(p) and p[j] in ('!', '^'):
                    j += 1
                if j < len(p) and p[j] == ']':
                    j += 1
                while j < len(p) and p[j] != ']':
                    if p[j] == '\\' and j + 1 < len(p):
                        j += 2
                    else:
                        j += 1
                if j < len(p):
                    tokens.append(('CHAR_CLASS', p[i:j+1]))
                    i = j + 1
                else:
                    tokens.append(('CHAR', '['))
            elif c == '\\':
                if i + 1 < len(p):
                    tokens.append(('CHAR', p[i + 1]))
                    i += 2
                else:
                    tokens.append(('CHAR', '\\'))
                    i += 1
            else:
                tokens.append(('CHAR', c))
            i += 1
        tokens.append(('END',))
        return tokens

    def match_segment(segment_tokens, segment, start_idx):
        i = start_idx
        while True:
            tok = segment_tokens[i]
            if tok[0] == 'END':
                return i == len(segment_tokens) - 1
            elif tok[0] == 'CHAR':
                if not segment:
                    return False
                if segment[0] == tok[1]:
                    segment = segment[1:]
                    i += 1
                else:
                    return False
            elif tok[0] == 'Q':
                if not segment:
                    return False
                segment = segment[1:]
                i += 1
            elif tok[0] == 'STAR':
                # Try matching 0 chars, then 1, etc.
                for k in range(len(segment) + 1):
                    new_seg = segment[k:]
                    if match_segment(segment_tokens, new_seg, i + 1):
                        return True
                return False
            elif tok[0] == 'CHAR_CLASS':
                if not segment:
                    return False
                char_str = segment[0]
                class_str = tok[1]
                # Parse class
                neg = False
                if class_str[1] == '!':
                    neg = True
                    class_content = class_str[2:-1]
                elif class_str[1] == '^':
                    neg = True
                    class_content = class_str[2:-1]
                else:
                    class_content = class_str[1:-1]
                
                matched = False
                j = 0
                while j < len(class_content):
                    if class_content[j] == '\\' and j + 1 < len(class_content):
                        if class_content[j + 1] == char_str:
                            matched = True
                        j += 2
                    elif j + 2 < len(class_content) and class_content[j + 1] == '-' and class_content[j + 2] != ']':
                        start_char = class_content[j]
                        end_char = class_content[j + 2]
                        if start_char == '\\' and len(start_char) > 1: # Should be handled by escape logic above
                            pass
                        if ord(start_char) <= ord(char_str) <= ord(end_char):
                            matched = True
                        j += 3
                    else:
                        if class_content[j] == '\\':
                            if j + 1 < len(class_content) and class_content[j + 1] == char_str:
                                matched = True
                            j += 2
                        else:
                            if class_content[j] == char_str:
                                matched = True
                            j += 1
                
                if neg:
                    if matched:
                        return False
                else:
                    if not matched:
                        return False
                segment = segment[1:]
                i += 1
            elif tok[0] == 'STAR_STAR':
                # Match zero or more whole segments
                # We are currently in a segment list, so we need to look ahead
                # This function is called for a single segment, but STAR_STAR matches multiple segments
                # We need a different approach for STAR_STAR
                pass
        return False

    def solve(tokens, segments):
        if not tokens:
            return len(segments) == 0
        if not segments:
            return tokens == [('END',)]
        
        tok = tokens[0]
        rest_tokens = tokens[1:]
        
        if tok[0] == 'STAR_STAR':
            # Try matching zero segments
            if solve(rest_tokens, segments):
                return True
            # Try matching one segment and recurse
            if len(segments) > 0:
                # Check if the next token in rest_tokens is also STAR_STAR
                # Actually, STAR_STAR matches *any* number of segments
                # So we consume one segment and try again with the same STAR_STAR token
                # But we must ensure we don't consume a STAR_STAR from the pattern as a literal
                # The pattern has already been tokenized.
                # So we try to match the current segment against the rest of the pattern
                # Wait, STAR_STAR matches the current segment AND potentially more, then we continue with rest_tokens
                # But we can also match the current segment as part of the STAR_STAR expansion
                # Let's try matching the current segment against the rest of the pattern (which might start with STAR)
                # No, STAR_STAR matches whole segments.
                # So we can match 0 segments: solve(rest_tokens, segments)
                # Or we can match 1 segment: we need to match the first segment of the path against the pattern?
                # No, STAR_STAR is a wildcard for segments.
                # So we can skip the current segment and try to match the rest of the segments with the rest of the tokens
                # But we also need to consider that the STAR_STAR might match multiple segments.
                # This is equivalent to: solve(rest_tokens, segments[1:]) OR solve([tok] + rest_tokens, segments[1:])
                # But we must be careful not to create infinite recursion.
                # Actually, the standard way is:
                # 1. Match 0 segments: solve(rest_tokens, segments)
                # 2. Match 1 or more: we consume one segment and try to match the rest of the tokens against the rest of the segments
                #    But the STAR_STAR can also match the current segment and continue matching the rest of the tokens?
                #    No, STAR_STAR matches segments. Once we decide to consume a segment, we are still in the STAR_STAR state
                #    until we decide to stop matching segments for this STAR_STAR.
                #    So we can either:
                #    - Stop matching segments now (solve(rest_tokens, segments))
                #    - Consume one segment and try again with the same STAR_STAR token (solve(tokens, segments[1:]))
                #    This covers all cases.
                if solve(tokens, segments[1:]):
                    return True
                # Also need to check if we can stop matching segments now
                if solve(rest_tokens, segments):
                    return True
                return False
            else:
                return False
        
        if tok[0] == 'STAR':
            # Matches zero or more chars in the current segment
            # We need to match the current segment against the rest of the tokens
            # We try matching 0 chars, then 1, etc.
            current_segment = segments[0]
            for i in range(len(current_segment) + 1):
                remaining_segment = current_segment[i:]
                if remaining_segment == '' and len(segments) == 1 and len(rest_tokens) == 1 and rest_tokens[0][0] == 'END':
                    return True
                if remaining_segment == '':
                    if solve(rest_tokens, segments[1:]):
                        return True
                else:
                    if solve(rest_tokens, [remaining_segment] + segments[1:]):
                        return True
            return False
        
        # For CHAR, Q, CHAR_CLASS
        current_segment = segments[0]
        remaining_segments = segments[1:]
        
        if tok[0] == 'CHAR':
            if not current_segment:
                return False
            if current_segment[0] == tok[1]:
                if current_segment[1:] == '' and len(remaining_segments) == 0 and len(rest_tokens) == 1 and rest_tokens[0][0] == 'END':
                    return True
                return solve(rest_tokens, [current_segment[1:]] + remaining_segments) if current_segment[1:] != '' else solve(rest_tokens, remaining_segments)
            return False
        elif tok[0] == 'Q':
            if not current_segment:
                return False
            if current_segment[0] == '/':
                return False
            if current_segment[1:] == '' and len(remaining_segments) == 0 and len(rest_tokens) == 1 and rest_tokens[0][0] == 'END':
                return True
            return solve(rest_tokens, [current_segment[1:]] + remaining_segments) if current_segment[1:] != '' else solve(rest_tokens, remaining_segments)
        elif tok[0] == 'CHAR_CLASS':
            if not current_segment:
                return False
            char_str = current_segment[0]
            if char_str == '/':
                return False
            class_str = tok[1]
            neg = False
            if class_str[1] == '!':
                neg = True
                class_content = class_str[2:-1]
            elif class_str[1] == '^':
                neg = True
                class_content = class_str[2:-1]
            else:
                class_content = class_str[1:-1]
            
            matched = False
            j = 0
            while j < len(class_content):
                if class_content[j] == '\\' and j + 1 < len(class_content):
                    if class_content[j + 1] == char_str:
                        matched = True
                    j += 2
                elif j + 2 < len(class_content) and class_content[j + 1] == '-' and class_content[j + 2] != ']':
                    start_char = class_content[j]
                    end_char = class_content[j + 2]
                    if ord(start_char) <= ord(char_str) <= ord(end_char):
                        matched = True
                    j += 3
                else:
                    if class_content[j] == '\\':
                        if j + 1 < len(class_content) and class_content[j + 1] == char_str:
                            matched = True
                        j += 2
                    else:
                        if class_content[j] == char_str:
                            matched = True
                        j += 1
            
            if neg:
                if matched:
                    return False
            else:
                if not matched:
                    return False
            
            if current_segment[1:] == '' and len(remaining_segments) == 0 and len(rest_tokens) == 1 and rest_tokens[0][0] == 'END':
                return True
            return solve(rest_tokens, [current_segment[1:]] + remaining_segments) if current_segment[1:] != '' else solve(rest_tokens, remaining_segments)
        
        return False

    if not pattern and not path:
        return True
    if not pattern or not path:
        return False
    
    tokens = parse_pattern(pattern)
    segments = path.split('/')
    
    # If pattern ends with / and path doesn't end with /, it might not match?
    # The problem says "whole path must match", and segments are separated by '/'
    # If path is "a/b", segments are ["a", "b"]
    # If pattern is "a/b", tokens should match ["a", "b"]
    # If path is "a/b/", segments would be ["a", "b", ""]
    # But split('/') on "a/b/" gives ["a", "b", ""]
    # We need to handle trailing slashes?
    # The problem says "Paths are separated by '/'". Usually this implies no trailing slash unless specified.
    # But let's assume standard split behavior.
    # If the pattern has a trailing slash, it implies a trailing segment.
    # Let's check if the path ends with '/'
    if path.endswith('/') and not path.endswith('//'):
        # It's a directory? Or just a trailing slash
        # If the pattern ends with '/', it expects a trailing segment
        # If the path ends with '/', it has an empty segment at the end
        pass
    elif not path.endswith('/') and pattern.endswith('/'):
        # Pattern expects a trailing segment, path doesn't have it
        # This should fail unless the last pattern token is STAR or STAR_STAR?
        # But if the pattern ends with '/', the last token might be CHAR('/') or similar
        # Actually, our parser will create a CHAR token for '/'
        # So if path doesn't end with '/', and pattern ends with '/', it will fail
        # This is correct.
        pass
    
    # Edge case: if path is empty string, segments is ['']
    # If pattern is empty, it matches empty path?
    # The problem says "whole path must match"
    # If path is "", segments = [""]
    # If pattern is "", tokens = [('END',)]
    # solve will check if tokens match segments
    # If tokens is [('END',)] and segments is [''], it should return False because len(segments) != 0
    # But wait, if path is "", it means no segments?
    # Let's check split behavior: "".split('/') -> ['']
    # So if path is "", segments = ['']
    # If pattern is "", it should match only if path is ""?
    # But the problem says "Paths are separated by '/'". An empty path has no segments.
    # Let's handle empty path explicitly.
    if path == '':
        return pattern == ''
    
    return solve(tokens, segments)