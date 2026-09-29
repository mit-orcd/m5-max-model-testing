```python
def match(pattern, path):
    def _compile(p_idx, p_len, segs, s_idx, s_len):
        # p_idx: current index in pattern string
        # p_len: length of pattern string
        # segs: list of path segments (split by '/')
        # s_idx: current segment index in segments
        # s_len: number of segments
        
        if p_idx == p_len and s_idx == s_len:
            return True
        
        if p_idx == p_len:
            return False
        
        if s_idx == s_len:
            # Pattern has more chars, but no path segments left.
            # Only match if the rest of pattern is empty (handled above)
            # or if we are matching an empty segment? No, segments are non-empty strings from split.
            # If path is empty string "" and pattern is "" -> True.
            # If path is "" and pattern is "/" -> split gives ["", ""]? 
            # Let's rely on split behavior: "a/b".split('/') -> ["a", "b"]
            # "" -> [""]
            # To handle empty paths correctly:
            if s_len == 0:
                # If path is empty, pattern must be empty string to match.
                # But if pattern is just "*/" or similar?
                # Let's restructure to handle string directly for segments logic.
                return False
        
        p_char = pattern[p_idx]
        
        if p_char == '\\':
            if p_idx + 1 < p_len:
                literal = pattern[p_idx + 1]
                if s_idx < s_len:
                    seg = segs[s_idx]
                    if seg == literal:
                        return _compile(p_idx + 2, p_len, segs, s_idx + 1, s_len)
                    # Check if seg is multi-char? No, backslash matches literal char.
                    # But path segment is a string.
                    # Backslash matches the next character in pattern as literal.
                    # It matches the character in the path segment? 
                    # "A backslash escapes the next character so it is treated literally."
                    # So if pattern is "a\\b", it matches path segment "ab"? No.
                    # It matches "a" then "b" as characters?
                    # Wait, the problem says: "A backslash escapes the next character so it is treated literally."
                    # This applies to the pattern characters.
                    # Does it mean the pattern matches the string character by character?
                    # Yes, but the problem defines '*' and '?' as matching characters within a segment.
                    # So we need to match the pattern against the path string segment by segment.
                    # Let's change strategy: match pattern against the full path string, 
                    # but with segment boundaries enforced by '/' and ** logic.
                    pass
                return False
            else:
                return False

        if p_char == '*':
            if p_idx + 1 < p_len and pattern[p_idx + 1] == '*':
                # Double star logic
                # Matches zero or more whole segments.
                # We need to skip segments until we match the rest of the pattern.
                # The rest of the pattern after "**/" starts matching from s_idx (if we consume 0 segments)
                # or s_idx + 1, etc.
                
                # If pattern ends with "**" or "**/" or "**abc"
                # If we are at the end of pattern, matches anything.
                if p_idx + 2 == p_len:
                    return True
                
                # If pattern continues after "**/"
                # Check if next char is '/'
                if p_idx + 2 < p_len and pattern[p_idx + 2] == '/':
                    # "**/" matches empty segments or multiple segments.
                    # Effectively, we can match the rest of the pattern against any suffix of segments.
                    # We try matching the rest of the pattern starting from s_idx, s_idx+1, ...
                    rest_pattern = pattern[p_idx + 3:] # skip "**/"
                    for i in range(s_idx, s_len + 1):
                        if _compile(rest_pattern, len(rest_pattern), segs, i, s_len):
                            return True
                    return False
                else:
                    # Case like "a**b" (no slash after **).
                    # This is tricky. In glob, "**" usually implies directory traversal.
                    # If there is no slash after, does it behave like *?
                    # Standard glob: "**" matches zero or more segments.
                    # If followed by non-slash, it's ambiguous.
                    # Usually "**" must be followed by "/" or end of pattern to be special.
                    # If pattern is "a**b", and path is "axb", does it match?
                    # Let's assume standard behavior: "**" matches segments only if followed by "/" or end.
                    # If not followed by "/", treat as '*'? Or fail?
                    # The problem says: "A path segment that is exactly '**' matches zero or more whole segments".
                    # This implies the pattern segment itself is "**".
                    # So we are matching segment by segment.
                    # Let's re-parse the pattern into segments.
                    return False # Fallback to char-by-char logic if not handled by segment logic
            
            # Single '*' logic
            # Matches zero or more characters in current segment, but never '/'.
            # We need to match the rest of the pattern against the current segment,
            # trying all possible split points.
            if s_idx < s_len:
                seg = segs[s_idx]
                # Try matching pattern[p_idx:] against seg[0:i] + seg[i:]?
                # We iterate over how many characters of the current segment match the '*'.
                # The '*' can match 0..len(seg) characters.
                # But we must ensure we don't cross the segment boundary.
                # If we match the whole segment, we move to next segment.
                # If we match part, we stay in current segment.
                
                # Optimization: if pattern continues with '/', then '*' must match the whole segment?
                # No, '*' matches characters. If next char is '/', then '*' matches up to '/'.
                # Wait, '*' never matches '/'. So if pattern has "**", that's different.
                # If pattern has "*", it matches chars in segment.
                # If the next char in pattern is '/', then '*' matches the current segment entirely?
                # No, '*' matches zero or more chars. If the next char in pattern is '/', 
                # the '*' must consume the rest of the segment (or part of it), 
                # and then the pattern '/' must match the segment end?
                # No, segments are separated by '/'. The '/' in pattern matches the '/' separator.
                # So if pattern is "a*b/c", and path is "axb/y/c".
                # "a*b" matches "axb". Then "/" matches separator. Then "c" matches "c".
                
                # Let's try matching the '*' against the current segment characters.
                # p_idx points to '*'.
                # We need to match pattern[p_idx+1:] against the rest of the path.
                # The '*' can match 0 to k characters in segs[s_idx].
                # If it matches k characters, then the next char in pattern must match the (k+1)th char of seg,
                # or if k == len(seg), we move to next segment.
                
                # Actually, if pattern[p_idx+1] is '/', then '*' matches the entire segment (or part, but then '/' must match).
                # Since '/' is not in segment, '*' must match the whole segment if we want to match the next '/'.
                # Wait, if pattern is "a*b", and path is "ab". Matches.
                # If pattern is "a*b/", and path is "ab/". Matches.
                # If pattern is "a*b/c", path is "ab/d/c".
                # '*' matches "ab". Then '/' matches '/'. Then "c" matches "c".
                
                # So we iterate over the number of characters '*' consumes from the current segment.
                # Let's say it consumes c characters.
                # Then the remaining pattern must match the rest of the current segment (c+1 to end) + next segments.
                # OR if c == len(seg), the remaining pattern must match the next segments.
                
                # But this is O(N^2) or more.
                # Better approach: match pattern recursively.
                
                # If pattern[p_idx+1] is '/':
                #   '*' must match the entire segment? Not necessarily, could match part and then '/' fails?
                #   No, if '*' matches part, then next char in pattern is '/', which doesn't match the next char in segment.
                #   So if next char in pattern is '/', '*' MUST match the whole segment.
                #   Exception: if path segment is empty? Segments from split are never empty unless path starts with /?
                #   "a/b".split('/') -> ["a", "b"].
                #   "/a/b".split('/') -> ["", "a", "b"]. Empty segments possible.
                
                # If pattern[p_idx+1] is not '/':
                #   '*' matches some chars, then pattern continues matching chars in segment.
                
                # Let's implement a recursive matcher that handles segments.
                return _match_segment(p_idx, p_len, segs, s_idx, s_len)
            else:
                return False

        if p_char == '?':
            if s_idx < s_len:
                seg = segs[s_idx]
                if len(seg) > 0:
                    # Match first char? No, '?' matches exactly one character.
                    # But we are matching segment by segment.
                    # We need to match the pattern against the segment string.
                    # '?' matches one char in segment.
                    # If pattern has "a?b", and segment is "acb", matches.
                    # If pattern has "a?b", and segment is "ab", fails.
                    # So we need to match pattern against segment string, not just segment equality.
                    pass
                return False
            else:
                return False
        
        # If p_char is a literal or part of a class
        # If next char is '/', then we must match the whole segment with the pattern prefix.
        # If not, we match character by character within the segment.
        
        # Let's restart with a more robust segment-aware logic.
        return False

    # Split path into segments
    if path == "":
        segments = []
    else:
        segments = path.split('/')
        # If path starts with '/', first element is empty.
        # If path is just "/", segments = ["", ""]?
        # "a//b".split('/') -> ["a", "", "b"].
        # We need to handle empty segments if they exist in path.
        # The problem says "Paths are separated by '/'".
        # "a/b" -> segments ["a", "b"]
        # "/a" -> segments ["", "a"]?
        # Usually glob treats "/" as separator.
        # Let's assume standard split.
    
    # Handle pattern splitting? No, pattern has wildcards inside segments.
    # We need to match pattern against the segments list.
    # But pattern wildcards '*' and '?' work on characters within segments.
    # So we need to match pattern string against path string, but respecting segment boundaries for '/' and '**'.
    
    # Let's do a recursive function that takes pattern string and path string,
    # but tracks if we are inside a segment or at a slash.
    
    # Actually, simpler: split pattern into "tokens" that are segments or special markers?
    # No, pattern can be "a*?b/c".
    
    # Let's write a direct recursive function that iterates through pattern and path.
    # State: p_idx, path_str.
    # If we encounter '/', it must match exactly one '/'.
    # If we encounter '**', it matches zero or more whole segments.
    # If we encounter '?' or '*', it matches chars within a segment.
    
    # We need to handle the path string split by '/' to respect segment boundaries.
    # Let's use the split list.
    # But we need to match pattern chars against segment chars.
    
    # Let's try a different approach:
    # Convert pattern into a regex? No, must implement logic.
    # Implement a recursive matcher.
    
    def _match(p_idx, path_str):
        p_len = len(pattern)
        # Skip slashes in pattern? No, pattern has slashes.
        # We need to match pattern against path_str, but with segment logic.
        
        # If pattern contains '/', we split pattern into parts?
        # Not necessarily, because '*' can span? No, '*' never matches '/'.
        # So pattern is a sequence of segments separated by '/'.
        # Except '**' which can span multiple segments.
        
        # Let's split pattern by '/' but keep track of '*' and '**'.
        # Actually, we can just process char by char.
        
        # If we hit '/', we must consume a '/' from path.
        # If we hit '**', we consume segments.
        # If we hit '?', '*', we match chars in current segment.
        
        # Let's iterate.
        
        # If p_idx reaches end:
        #   path_str must be empty or consist of only slashes?
        #   If path_str is empty, True.
        #   If path_str is "///", split gives empty segments?
        #   If path_str is not empty, False.
        
        # Check for '**'
        if p_idx + 1 < p_len and pattern[p_idx] == '*' and pattern[p_idx+1] == '*':
            # Match zero or more segments.
            # The rest of the pattern after "**" must match the rest of the path.
            # "**" can match 0 segments, 1 segment, etc.
            # If pattern ends with "**", matches anything.
            if p_idx + 2 == p_len:
                return True
            
            # If pattern continues, it must be after "**"
            # Check if pattern[p_idx+2] is '/'
            if p_idx + 2 < p_len and pattern[p_idx+2] == '/':
                # "**/" matches zero or more segments.
                # The rest of the pattern is pattern[p_idx+3:]
                rest_p = pattern[p_idx+3:]
                # Try matching rest_p against the rest of the path segments.
                # We can skip any number of segments.
                # Path segments:
                # If path_str is "a/b/c", segments = ["a", "b", "c"]
                # Try matching rest_p against ["a", "b", "c"], ["b", "c"], ["c"], []
                # But we must use the segment matching logic for the remaining pattern.
                
                # Let's split the path_str by '/' to get segments.
                # But we need to handle empty segments correctly.
                # Let's use the split list from the top level?
                # No, we need to pass the segments list.
                pass
            
            # If not followed by '/', treat as '*'? Or fail?
            # Standard glob: "**" only matches segments if followed by '/' or end.
            # If followed by non-slash, it's treated as "**" matching characters? No.
            # Problem says: "A path segment that is exactly '**' matches zero or more whole segments"
            # This implies the pattern segment is "**".
            # So we must split pattern into segments based on '/'?
            # But '*' inside segment is allowed.
            # So pattern "a**b/c" is invalid? Or "**b" is a segment?
            # "A path segment that is exactly '**'" -> implies the segment string in pattern is "**".
            # So pattern "a**/b" -> segments: "a**", "b".
            # Pattern "a/**/b" -> segments: "a", "**", "b".
            # Pattern "a**b/c" -> segments: "a**b", "c"?
            # If "a**b" is a segment, then "**" inside it is not "exactly '**'".
            # So it's treated as literal '*'? No, '*' is wildcard.
            # "A path segment that is exactly '**'" suggests we split by '/' and check if segment == "**".
            
            # Let's rewrite: Split pattern by '/' into parts.
            # parts = pattern.split('/')?
            # But "**" might be "a**/b". Split -> ["a**", "b"].
            # "a**" is not "**". So it's a normal segment with wildcards.
            # "a/**/b". Split -> ["a", "**", "b"]. "b" matches "b". "**" matches segments.
            
            # So strategy:
            # 1. Split pattern by '/' to get pattern_segments.
            # 2. Split path by '/' to get path_segments.
            # 3. Match pattern_segments against path_segments.
            # 4. If pattern_segment == "**", it matches zero or more path_segments.
            # 5. If pattern_segment is not "**", match it as a string against the corresponding path_segment.
            
            # But wait, what if pattern is "a*b/c"?
            # Split -> ["a*b", "c"].
            # "a*b" matches "ab" in path? Yes.
            # What if pattern is "a**/b"?
            # Split -> ["a**", "b"].
            # "a**" is not "**". So it's a segment.
            # But "**" inside "a**" should match characters?
            # Problem: "A path segment that is exactly '**' matches zero or more whole segments".
            # This implies if the segment is literally "**", it's special.
            # If it's "a**", it's a normal segment, and "**" inside is treated as '*' and '*'?
            # Or does "**" inside a segment count as the special case?
            # "exactly '**'" means the segment string must be equal to "**".
            # So "a**" is not "**". So it's a normal segment.
            # Then how does "**" behave inside "a**"?