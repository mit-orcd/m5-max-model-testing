def match(pattern, path):
    """
    Returns True if a glob pattern matches a whole path.
    Paths are separated by '/'.
    '?' matches exactly one character but never '/'.
    '*' matches zero or more characters but never '/'.
    '**' matches zero or more whole segments.
    Square brackets [...] are a character class.
    Backslash escapes the next character.
    """
    
    def parse_pattern(pat):
        """
        Parses the pattern into a list of tokens.
        Each token is either:
        - ('literal', char)
        - ('wildcard', None)  for '?'
        - ('star', None)      for '*' (single)
        - ('star_star', None) for '**'
        - ('class', chars, negated)
        
        Segments are handled by the '/' separator logic in the matcher, 
        but to support '**', we need to know where segments start and end.
        
        Actually, it's easier to process the pattern recursively or iteratively 
        segment by segment, but '**' spans segments.
        
        Let's convert the pattern into a list of "segments" and "star_star" markers.
        A segment consists of tokens that do not cross '/'.
        '**' is a special token that matches between segments.
        """
        tokens = []
        i = 0
        n = len(pat)
        
        while i < n:
            c = pat[i]
            if c == '/':
                tokens.append(('slash', None))
                i += 1
            elif c == '\\':
                if i + 1 < n:
                    tokens.append(('literal', pat[i+1]))
                    i += 2
                else:
                    # Trailing backslash? Treat as literal? 
                    # Standard glob usually treats trailing backslash as literal.
                    tokens.append(('literal', c))
                    i += 1
            elif c == '[':
                # Parse character class
                j = i + 1
                negated = False
                # Check for negation
                if j < n and pat[j] == '!':
                    negated = True
                    j += 1
                # Find closing bracket
                # Note: ']' immediately after '[' or '[!' closes the class in some implementations,
                # but standard glob usually requires ']' to be escaped or first char.
                # Let's assume standard behavior: find ']' that is not escaped.
                chars = []
                while j < n and pat[j] != ']':
                    if pat[j] == '\\':
                        if j + 1 < n:
                            chars.append(pat[j+1])
                            j += 2
                        else:
                            chars.append(pat[j])
                            j += 1
                    else:
                        # Check for range
                        if (j + 2 < n and pat[j+1] == '-' and pat[j+2] != ']'):
                            start_char = pat[j]
                            end_char = pat[j+2]
                            # Handle escape in range
                            # If start_char was escaped, it's just the char.
                            # Let's just append the characters/ranges.
                            # For simplicity, we store the string chunk and parse it later?
                            # Or store (start, end) tuples.
                            chars.append((start_char, end_char))
                            j += 3
                        else:
                            chars.append(pat[j])
                            j += 1
                if j < n:
                    j += 1 # Skip ']'
                    tokens.append(('class', chars, negated))
                    i = j
                else:
                    # No closing bracket, treat as literals
                    tokens.append(('literal', c))
                    i += 1
            elif c == '*':
                # Check for '**'
                if i + 1 < n and pat[i+1] == '*':
                    tokens.append(('star_star', None))
                    i += 2
                    # Skip any trailing slash after '**' if it's a segment separator?
                    # Actually, '**' matches zero or more directories.
                    # So 'a/**/b' -> segment 'a', star_star, segment 'b'.
                    # 'a/**' -> segment 'a', star_star.
                    # We need to handle slashes around '**' carefully.
                    # If there is a slash after '**', it's consumed by the next segment logic?
                    # Let's just add the token. The matcher will handle slashes.
                    if i < n and pat[i] == '/':
                        tokens.append(('slash', None))
                        i += 1
                else:
                    tokens.append(('star', None))
                    i += 1
            else:
                tokens.append(('literal', c))
                i += 1
        return tokens

    def match_segment_segment(segment_tokens, path_segment):
        """
        Matches a list of tokens (from a single path segment) against a path segment string.
        Returns True if they match.
        '?' and '*' do not match '/'. Since we are processing segment by segment,
        '/' is already separated.
        """
        if not segment_tokens:
            return path_segment == ''
        
        pat_iter = iter(segment_tokens)
        path_idx = 0
        path_len = len(path_segment)
        
        # We can use a recursive approach or iterative with backtracking.
        # Since segments are simple, let's use recursion.
        
        def rec(pt_idx, pi):
            if pt_idx >= len(segment_tokens):
                return pi == path_len
            
            token = segment_tokens[pt_idx]
            t_type = token[0]
            
            if t_type == 'literal':
                char = token[1]
                if pi < path_len and path_segment[pi] == char:
                    return rec(pt_idx + 1, pi + 1)
                else:
                    return False
            elif t_type == 'class':
                chars = token[1]
                negated = token[2]
                if pi < path_len:
                    char = path_segment[pi]
                    # Check if char is in class
                    in_class = False
                    for item in chars:
                        if isinstance(item, tuple):
                            start, end = item
                            if start <= char <= end:
                                in_class = True
                                break
                        else:
                            if item == char:
                                in_class = True
                                break
                    if negated:
                        if in_class:
                            return False
                        else:
                            return rec(pt_idx + 1, pi + 1)
                    else:
                        if in_class:
                            return rec(pt_idx + 1, pi + 1)
                        else:
                            return False
                else:
                    return False
            elif t_type == 'wildcard':
                if pi < path_len:
                    return rec(pt_idx + 1, pi + 1)
                else:
                    return False
            elif t_type == 'star':
                # '*' matches zero or more characters (not '/')
                # Try matching 0, 1, 2... chars
                remaining_tokens = segment_tokens[pt_idx+1:]
                for k in range(pi, path_len + 1):
                    if rec(len(remaining_tokens), 0) is False: # This logic is messy
                        pass
                    # Better: try matching the rest of the pattern against the rest of the path
                    # starting from k
                    if _match_tokens(remaining_tokens, path_segment[k:]):
                        return True
                return False
            return False
            
        # Helper to match a list of tokens against a string
        def _match_tokens(tokens, text):
            if not tokens:
                return text == ''
            
            # Check for star at the beginning of tokens
            # If the first token is a star, we can try skipping 0..len(text) chars
            # But we already handled star in the main loop above? 
            # No, _match_tokens is called recursively for the rest.
            
            # Let's rewrite _match_tokens to be robust
            return _match_tokens_impl(tokens, text)

        def _match_tokens_impl(tokens, text):
            if not tokens:
                return text == ''
            
            token = tokens[0]
            rest_tokens = tokens[1:]
            t_type = token[0]
            
            if t_type == 'star':
                # Try matching 0 to len(text) characters
                for k in range(len(text) + 1):
                    if _match_tokens_impl(rest_tokens, text[k:]):
                        return True
                return False
            elif t_type == 'star_star':
                # Star_star should not appear inside a segment. 
                # It appears between segments.
                # This function is for segment matching, so this case shouldn't happen 
                # unless the pattern is malformed for a single segment.
                return False
            elif t_type == 'slash':
                return False # Slashes are handled by segment splitting
            elif t_type == 'literal':
                if text and text[0] == token[1]:
                    return _match_tokens_impl(rest_tokens, text[1:])
                return False
            elif t_type == 'wildcard':
                if text:
                    return _match_tokens_impl(rest_tokens, text[1:])
                return False
            elif t_type == 'class':
                if text:
                    char = text[0]
                    chars = token[1]
                    negated = token[2]
                    in_class = False
                    for item in chars:
                        if isinstance(item, tuple):
                            start, end = item
                            if start <= char <= end:
                                in_class = True
                                break
                        else:
                            if item == char:
                                in_class = True
                                break
                    if negated:
                        if in_class:
                            return False
                        else:
                            return _match_tokens_impl(rest_tokens, text[1:])
                    else:
                        if in_class:
                            return _match_tokens_impl(rest_tokens, text[1:])
                        else:
                            return False
                return False
            return False

        return _match_tokens_impl(segment_tokens, path_segment)

    def match_full(pattern_tokens, path):
        """
        Matches the full token list against the full path string.
        """
        # Split path by '/'
        # However, '**' spans segments.
        # We need to split the pattern_tokens into segments based on 'slash' and 'star_star'.
        
        # Group tokens into segments.
        # A segment is a list of tokens between slashes (or start/end).
        # 'star_star' acts as a bridge.
        
        segments = []
        current_segment = []
        for token in pattern_tokens:
            if token[0] == 'slash':
                segments.append(current_segment)
                current_segment = []
            elif token[0] == 'star_star':
                # This marks a special matching point.
                # It matches zero or more complete segments.
                # We store it separately.
                segments.append(('star_star',))
                current_segment = []
            else:
                current_segment.append(token)
        segments.append(current_segment)
        
        # Now match segments against path.split('/')
        # But wait, path.split('/') creates empty strings for leading/trailing/consecutive slashes.
        # We need to handle path splitting carefully.
        
        # Let's use a recursive matcher on the segments list and path string.
        
        def rec_segment_match(pat_segments, path_str):
            if not pat_segments:
                return path_str == ''
            
            current_pat = pat_segments[0]
            rest_pat = pat_segments[1:]
            
            # If current_pat is a 'star_star' marker
            if current_pat == ('star_star',):
                # '**' matches zero or more segments.
                # Case 1: Matches zero segments.
                # The next pat segment (rest_pat[0]) must match the current path segment.
                # But wait, '**' can match multiple segments.
                # So we try matching rest_pat against:
                # 1. The rest of the path (zero segments skipped)
                # 2. A path with one segment removed (one segment skipped)
                # 3. A path with two segments removed, etc.
                
                # If rest_pat is empty, then '**' must consume the rest of the path.
                if not rest_pat:
                    # Does '**' match empty string? No, it matches segments.
                    # If path is empty, does ** match? 
                    # 'a/**' matches 'a'.
                    # So if rest_pat is empty, we just need to check if the previous part matched.
                    # But we are matching from the current point.
                    # If path_str is empty, and we have '**' at the end, it matches.
                    return path_str == ''
                
                # Try matching rest_pat against the rest of the path (skipping 0 segments)
                if rec_segment_match(rest_pat, path_str):
                    return True
                
                # Try matching rest_pat against the path with the first segment removed (skipping 1 segment)
                # Find the first '/'
                slash_idx = path_str.find('/')
                if slash_idx != -1:
                    # Remove the first segment (up to and including the slash)
                    remaining_path = path_str[slash_idx+1:]
                    if rec_segment_match(pat_segments, remaining_path):
                        return True
                else:
                    # No more slashes, '**' consumes the rest if rest_pat matches empty path?
                    # If rest_pat is empty, '**' matches the rest.
                    if not rest_pat:
                        return True
                    # If rest_pat is not empty, we can't skip beyond the end unless rest_pat matches empty.
                    # But we already tried rest_pat against the full path (which includes the last segment).
                    # We need to try skipping the last segment too?
                    # If path_str has no slash, it's a single segment.
                    # '**' can match this single segment if rest_pat matches empty.
                    if rest_pat == ('star_star',) or (rest_pat and rest_pat[0] == ('star_star',)):
                         # This is handled by the recursive call with empty path?
                         pass
                    # Let's handle the case where '**' consumes the last segment.
                    # If we remove the last segment, the path becomes empty (or just slashes).
                    # Actually, if there is no slash, the whole path is one segment.
                    # If '**' matches it, then rest_pat must match empty string.
                    if not rest_pat:
                        return True
                    # If rest_pat is not empty, it must match the rest.
                    # But we already called rec_segment_match(rest_pat, path_str).
                    # So we just need to handle the case where '**" consumes the current segment and passes to rest_pat.
                    # This is effectively: skip the current segment (all of it), and match rest_pat against empty.
                    if rec_segment_match(rest_pat, ''):
                        return True
                
                return False

            else:
                # Current_pat is a list of tokens for a single segment.
                # We need to match this segment against the next segment of the path.
                
                # Find the next '/' in path_str
                slash_idx = path_str.find('/')
                if slash_idx != -1:
                    current_segment_str = path_str[:slash_idx]
                    remaining_path = path_str[slash_idx+1:]
                else:
                    current_segment_str = path_str
                    remaining_path = ''
                
                # Match the current segment tokens against the current segment string
                if match_segment_segment(current_pat, current_segment_str):
                    # If successful, match the rest of the pattern against the rest of the path
                    return rec_segment_match(rest_pat, remaining_path)
                else:
                    return False

        return rec_segment_match(segments, path)

    tokens = parse_pattern(pattern)
    return match_full(tokens, path)