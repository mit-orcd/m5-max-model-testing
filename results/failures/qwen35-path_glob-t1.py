```python
def match(pattern, path):
    def parse_pattern(p, idx):
        # Returns (is_star, next_idx) for the current segment
        # 'is_star' is True if this segment is '**' (matches whole segments)
        # otherwise it's a standard pattern segment
        pass

    def compile_pattern(pattern):
        # Compile the pattern into a list of parts
        # Each part is a tuple: ('literal', char) | ('char_class', char_class_str) | ('star', 0) | ('star', 1)
        # '*' (depth 0) matches chars except '/'
        # '**' (depth 1) matches whole segments (including empty)
        parts = []
        i = 0
        n = len(pattern)
        
        while i < n:
            char = pattern[i]
            
            if char == '\\':
                if i + 1 < n:
                    parts.append(('literal', pattern[i+1]))
                    i += 2
                else:
                    parts.append(('literal', '\\'))
                    i += 1
            elif char == '?':
                parts.append(('char', '?'))
                i += 1
            elif char == '*':
                # Check for double star
                if i + 1 < n and pattern[i+1] == '*':
                    parts.append(('star', 1)) # matches whole segments
                    i += 2
                    # Skip any slashes after ** to handle 'a/**/b' correctly? 
                    # Actually the spec says '**' matches zero or more whole segments.
                    # We need to consume the pattern carefully.
                    # If we have 'a/**/b', the next char after '**' might be '/'
                    # But our logic will handle the slash in the path processing.
                    continue
                else:
                    parts.append(('star', 0)) # matches chars except '/'
                    i += 1
            elif char == '[':
                # Parse character class
                end = pattern.find(']', i)
                if end == -1:
                    # No closing bracket, treat as literal
                    parts.append(('literal', char))
                    i += 1
                else:
                    class_str = pattern[i+1:end]
                    parts.append(('char_class', class_str))
                    i = end + 1
            else:
                parts.append(('literal', char))
                i += 1
        
        return parts

    def match_segment(segment, segment_parts, idx):
        # Matches segment against segment_parts[idx:]
        # Returns True if the whole segment matches the rest of the pattern
        # segment_parts is the list of parts for a single path segment
        # We need to handle the recursion for '**' properly
        
        # If segment is empty? No, segments are non-empty strings between slashes
        # But '**' can match empty segment count.
        
        p_idx = idx
        s_idx = 0
        n = len(segment)
        
        while p_idx < len(segment_parts):
            part = segment_parts[p_idx]
            type_, val = part
            
            if type_ == 'star':
                if val == 1:
                    # Matches zero or more whole segments
                    # We treat this by trying to match 0 segments (skip this part)
                    # or match 1 segment (consume one whole segment from path)
                    # But wait, the pattern is compiled globally.
                    # Let's restructure the matching logic to be recursive.
                    return match_recursive(path, segment_parts, p_idx, 0)
                else:
                    # '*' matches chars except '/'
                    # Since we are matching a single segment string, no '/' exists inside.
                    # Just match against the string.
                    pass
            
            # If we are here, we are matching a standard part against the current segment string
            # This is getting complex. Let's switch to a recursive matcher on the whole path.
            return False # Fallback
        return True

    def match_recursive(path, parts, p_idx, s_idx):
        # path: remaining string of path
        # parts: compiled pattern list
        # p_idx: current index in parts
        # s_idx: current index in path
        
        if p_idx >= len(parts):
            return s_idx == len(path)
        
        part = parts[p_idx]
        type_, val = part
        
        if type_ == 'star' and val == 1:
            # '**' matches zero or more whole segments
            # Option 1: Match zero segments (skip this part, stay at same path segment)
            if match_recursive(path, parts, p_idx + 1, s_idx):
                return True
            
            # Option 2: Match one or more segments
            # Find the next '/'
            next_slash = path.find('/', s_idx)
            if next_slash == -1:
                # No more slashes, consume rest of path
                if match_segment(path[s_idx:], [parts[p_idx+1]], 0):
                     # Wait, we need to recurse properly.
                     # If we consume the rest of the path, we must check if the rest of the pattern matches the rest of the path.
                     # Actually, if we consume the rest of the path, the rest of the pattern must match the rest of the path?
                     # No, the rest of the pattern (after this '**') must match the rest of the path (which is empty).
                     # So if we consume the whole remaining path, the next part must match empty string?
                     # Let's simplify:
                     # If we consume the rest of the path, we are left with no path and maybe more pattern.
                     # So we check if the rest of the pattern matches empty string.
                     return match_recursive("", parts, p_idx + 1, 0)
                return False
            else:
                # Try consuming up to next slash
                # Segment is path[s_idx : next_slash]
                segment = path[s_idx : next_slash]
                # If we consume this segment, we need to match the next part of the pattern against the NEXT segment
                # But the next part might be another '*' or literal.
                # If the next part is a literal, it must match the segment we just consumed? No.
                # The logic is: ** matches segment 1, then we continue matching the rest of the pattern against the rest of the path.
                
                # If the next part in pattern is also '**', we can merge them.
                # If the next part is a normal pattern, we need to check if the segment matches the next pattern part?
                # No, the structure is: pattern ** pattern
                # If we consume one segment, the first part of the rest of the pattern must match the segment we just consumed?
                # No, the pattern is a sequence.
                # If we have 'a/**/b', and path is 'a/x/b'.
                # p_idx points to '**'.
                # We skip it (match 0 segments): path is 'a/x/b', parts start from '/b'.
                # We match 1 segment: consume 'x'. Now path is '/b'. parts start from '/b'.
                # The next part in pattern is '/' then 'b'.
                # So we need to handle slashes in the pattern list.
                
                # Let's re-implement the matching function to be simpler:
                # Convert path to segments.
                pass

    # Revised Approach: Segment-based matching
    segments = path.split('/') if path else []
    # If path is empty string, segments is [''].
    if path == '':
        segments = ['']
    else:
        segments = path.split('/')

    # But wait, if path starts with '/', split gives empty string at start.
    # The problem says "Paths are separated by '/'".
    # Let's assume standard paths.
    # If path is 'a/b', segments are ['a', 'b'].
    # If path is '/a/b', split gives ['', 'a', 'b'].
    # The spec says "A path segment that is exactly '**' matches zero or more whole segments".
    # This implies we are matching segments.
    
    # Let's rewrite the matching function to work on segments.
    
    pattern_parts = []
    i = 0
    n = len(pattern)
    
    while i < n:
        if pattern[i] == '\\':
            if i + 1 < n:
                pattern_parts.append(('literal', pattern[i+1]))
                i += 2
            else:
                pattern_parts.append(('literal', '\\'))
                i += 1
        elif pattern[i] == '?':
            pattern_parts.append(('char', '?'))
            i += 1
        elif pattern[i] == '*':
            if i + 1 < n and pattern[i+1] == '*':
                pattern_parts.append(('star', 1))
                i += 2
                # Handle potential slash after **? No, ** is a single token.
                # We just skip the slash in the pattern string if it exists?
                # The spec says 'a/**/b'. The slash is a separator.
                # So we need to consume slashes in the pattern loop?
                # No, the pattern loop should just parse tokens.
                # But slashes in the pattern are separators between segments.
                # So we need to handle slashes in the pattern logic.
                if i < n and pattern[i] == '/':
                    i += 1
            else:
                pattern_parts.append(('star', 0))
                i += 1
        elif pattern[i] == '[':
            end = pattern.find(']', i)
            if end == -1:
                pattern_parts.append(('literal', pattern[i]))
                i += 1
            else:
                class_str = pattern[i+1:end]
                pattern_parts.append(('char_class', class_str))
                i = end + 1
        elif pattern[i] == '/':
            # Separator. We need to handle segment matching.
            # But the pattern_parts list is linear.
            # We should just include '/' as a literal in the pattern_parts?
            # No, because '/' is a separator.
            # If we encounter '/' in pattern, it means we are moving to the next segment.
            # So we should force a segment boundary.
            # This complicates the linear list.
            # Let's just treat '/' as a literal character that must match '/' in path?
            # But '?' and '*' don't match '/'.
            # If we treat '/' as a literal, it works.
            # But the spec says "A path segment that is exactly '**' matches zero or more whole segments".
            # This implies the pattern is divided by '/'.
            # So 'a/**/b' is segments: 'a', '**', 'b'.
            # We should parse the pattern into segments.
            # If the pattern contains '/', we split by '/', but handle '**' carefully.
            pass
        
        # Let's restart the parsing to split by '/' properly.
        break

    # Correct Parsing Strategy:
    # Split pattern by '/', but be careful with '**'.
    # Actually, we can just iterate and treat '/' as a boundary.
    # But '?' and '*' cannot match '/'.
    # So if we encounter '/' in path, we must have a '/' in pattern.
    # So we can split the pattern string by '/' to get segments.
    # But 'a/**/b' -> ['a', '**', 'b']?
    # Yes.
    # What about 'a//*'? -> ['a', '', '*']? No, '//' is two slashes.
    # Let's just parse the pattern into a list of tokens, where '/' is a token that matches '/' in path.
    # But the spec says "A path segment that is exactly '**' matches zero or more whole segments".
    # This implies that if we have 'a/**/b', the pattern matches 'a' then zero or more segments, then 'b'.
    # So we should split the pattern by '/' to get segments, handling '**' as a special segment.
    
    # Let's parse the pattern into a list of segments.
    # Each segment is a pattern for one path segment.
    # If the pattern has '/', we split it.
    # But we need to handle the case where the pattern ends with '/'.
    
    # Let's do a recursive match on segments.
    
    # Parse pattern into segments
    # We need to handle the case where a segment is '**'.
    # Also handle the case where a segment is '*' (which matches chars in a segment).
    
    segments = []
    current_segment = []
    i = 0
    n = len(pattern)
    
    while i < n:
        if pattern[i] == '/':
            # End of current segment
            # Process current_segment
            seg_parts = []
            j = 0
            while j < len(current_segment):
                char = current_segment[j]
                if char == '\\':
                    if j + 1 < len(current_segment):
                        seg_parts.append(('literal', current_segment[j+1]))
                        j += 2
                    else:
                        seg_parts.append(('literal', '\\'))
                        j += 1
                elif char == '?':
                    seg_parts.append(('char', '?'))
                    j += 1
                elif char == '*':
                    if j + 1 < len(current_segment) and current_segment[j+1] == '*':
                        seg_parts.append(('star', 1))
                        j += 2
                        if j < len(current_segment) and current_segment[j] == '/':
                            j += 1
                    else:
                        seg_parts.append(('star', 0))
                        j += 1
                elif char == '[':
                    end = current_segment.find(']', j)
                    if end == -1:
                        seg_parts.append(('literal', char))
                        j += 1
                    else:
                        class_str = current_segment[j+1:end]
                        seg_parts.append(('char_class', class_str))
                        j = end + 1
                else:
                    seg_parts.append(('literal', char))
                    j += 1
            if seg_parts:
                segments.append(seg_parts)
            else:
                segments.append([]) # Empty segment (e.g. from //)
            current_segment = []
            i += 1
        elif pattern[i] == '\\':
            if i + 1 < n:
                current_segment.append(pattern[i+1])
                i += 2
            else:
                current_segment.append('\\')
                i += 1
        elif pattern[i] == '?':
            current_segment.append('?')
            i += 1
        elif pattern[i] == '*':
            if i + 1 < n and pattern[i+1] == '*':
                # Double star. Need to handle the rest of the pattern carefully.
                # If we have 'a/**/b', we split at '/'.
                # So 'a' is one segment, '**' is another segment.
                # We need to consume the next '/' to separate it.
                current_segment.append('*') # Will be handled as special
                current_segment.append('*') # Or just mark as double star?
                # Let's just append '**' as a string and parse later?
                # No, we are building segments.
                # Let's just append the characters and handle parsing later.
                # But we need to skip the slash after ** if it exists.
                # Actually, the split by '/' logic is better if we treat '**' as a token.
                # Let's change the loop to handle slashes as delimiters.
                pass
        else:
            current_segment.append(pattern[i])
            i += 1
    
    # This manual parsing is error-prone. Let's simplify.
    # Just split the pattern by '/' to get segments.
    # But we need to handle '**' correctly.
    # If we have 'a/**/b', split gives ['a', '**', 'b'].
    # If we have 'a/*', split gives ['a', '*'].
    # If we have 'a/**', split gives ['a', '**'].
    # If we have 'a//b', split gives ['a', '', 'b'].
    # If we have '**', split gives ['**'].
    
    # But wait, what if the pattern is 'a/**/b/c'?
    # Split gives ['a', '**', 'b', 'c'].
    # What if the pattern is 'a/**/b' and we split by '/'?
    # We get ['a', '**', 'b'].
    # This seems correct.
    
    # But we need to handle the case where the pattern ends with '/'.
    # If pattern is 'a/', split gives ['a', ''].
    # If pattern is '/a', split gives ['', 'a'].
    
    # Let's implement the split and then process each segment.
    
    # Handle the pattern string splitting by '/'
    # But we need to be careful about '**'.
    # If we have 'a/**/b', the '/' after '**' is a separator.
    # So we can just split by '/' and then process each part.
    # But we need to handle the case where a part is '**'.
    
    # Let's parse the pattern into a list of segments.
    # Each segment is a list of tokens.
    
    pattern_segments = []
    current_segment = []
    i = 0
    n = len(pattern)
    
    while i < n:
        if pattern[i] == '/':
            # Finish current segment
            if current_segment:
                # Parse current_segment
                seg_tokens = parse_segment(current_segment)
                pattern_segments.append(seg_tokens)
            else:
                # Empty segment (from // or / at start)
                pattern_segments.append([])
            current_segment = []
            i += 1
        elif pattern[i] == '\\':
            if i + 1 < n:
                current_segment.append(pattern[i+1])
                i += 2
            else:
                current_segment.append('\\')
                i += 1
        elif pattern[i] == '?':
            current_segment.append('?')
            i += 1
        elif pattern[i] == '*':
            if i + 1 < n and pattern[i+1] == '*':
                # Double star
                # Check if there is a slash after
                #