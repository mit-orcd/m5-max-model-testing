```python
def match(pattern, path):
    def compile_pattern(p):
        parts = []
        i = 0
        while i < len(p):
            c = p[i]
            if c == '*':
                if i + 1 < len(p) and p[i+1] == '*':
                    if i + 2 < len(p) and p[i+2] == '/':
                        parts.append(('star_star',))
                        i += 3
                        continue
                    elif i == 0 or p[i-1] == '/':
                        # Handle trailing ** or ** at start
                        parts.append(('star_star',))
                        i += 2
                        continue
                parts.append(('star',))
                i += 1
            elif c == '?':
                parts.append(('question',))
                i += 1
            elif c == '[':
                j = i + 1
                neg = False
                if j < len(p) and p[j] == '!':
                    neg = True
                    j += 1
                if j < len(p) and p[j] == ']':
                    j += 1
                while j < len(p) and p[j] != ']':
                    j += 1
                if j >= len(p):
                    parts.append(('literal', '['))
                    i += 1
                else:
                    char_class = p[i:j+1]
                    parts.append(('char_class', char_class, neg))
                    i = j + 1
            elif c == '\\':
                if i + 1 < len(p):
                    parts.append(('literal', p[i+1]))
                    i += 2
                else:
                    parts.append(('literal', '\\'))
                    i += 1
            else:
                parts.append(('literal', c))
                i += 1
        return parts

    def match_segment(segment, p_parts):
        idx = 0
        seg_len = len(segment)
        pat_idx = 0
        pat_len = len(p_parts)

        while pat_idx < pat_len:
            p_type = p_parts[pat_idx][0]
            
            if p_type == 'literal':
                if idx >= seg_len or segment[idx] != p_parts[pat_idx][1]:
                    return False
                idx += 1
                pat_idx += 1
            elif p_type == 'question':
                if idx >= seg_len or segment[idx] == '/':
                    return False
                idx += 1
                pat_idx += 1
            elif p_type == 'char_class':
                if idx >= seg_len:
                    return False
                ch = segment[idx]
                if ch == '/':
                    return False
                char_str = p_parts[pat_idx][1][1:-1] # Remove [ ]
                neg = p_parts[pat_idx][2]
                matched = False
                if len(char_str) > 0 and char_str[0] == '!':
                    # Negated class
                    char_str = char_str[1:]
                    if len(char_str) == 0 or (len(char_str) >= 2 and char_str[1] == ']'):
                        # Handle [!]]] or similar edge cases
                        pass
                    # Re-evaluate logic for negated class properly
                    # The char_str inside brackets might contain ranges
                    inner = p_parts[pat_idx][1][2:-1] # Remove [! and ]
                    if not inner:
                        matched = False
                    else:
                        # Check if char is in range
                        matched = False
                        j = 0
                        while j < len(inner):
                            if j + 2 < len(inner) and inner[j+1] == '-':
                                if inner[j] <= ch <= inner[j+2]:
                                    matched = True
                                    break
                                j += 3
                            else:
                                if inner[j] == ch:
                                    matched = True
                                    break
                                j += 1
                        # If negated, we want NOT matched
                        if neg:
                            if matched:
                                return False
                            else:
                                idx += 1
                                pat_idx += 1
                                continue
                        else:
                            if not matched:
                                return False
                            else:
                                idx += 1
                                pat_idx += 1
                                continue
                else:
                    # Non-negated class
                    inner = p_parts[pat_idx][1][1:-1]
                    if not inner:
                        matched = False
                    else:
                        matched = False
                        j = 0
                        while j < len(inner):
                            if j + 2 < len(inner) and inner[j+1] == '-':
                                if inner[j] <= ch <= inner[j+2]:
                                    matched = True
                                    break
                                j += 3
                            else:
                                if inner[j] == ch:
                                    matched = True
                                    break
                                j += 1
                    
                    if neg:
                        if matched:
                            return False
                        else:
                            idx += 1
                            pat_idx += 1
                    else:
                        if matched:
                            idx += 1
                            pat_idx += 1
                        else:
                            return False
            elif p_type == 'star':
                # Match 0 or more chars (no /)
                # Greedy match or backtrack? Since no / allowed, just consume non-/ chars
                start_idx = idx
                while idx < seg_len and segment[idx] != '/':
                    idx += 1
                pat_idx += 1
            elif p_type == 'star_star':
                # Matches zero or more whole segments
                # We are inside a segment, so this pattern part shouldn't happen here unless logic is wrong
                # Actually, star_star is handled at segment level, not char level.
                # If we are here, it means we are processing a single segment against a list of parts.
                # But star_star is a special token that spans segments.
                # So this branch should not be reached if we split correctly.
                return False
            else:
                return False
        
        return idx == seg_len

    def match_path(path_parts, p_parts):
        if not path_parts and not p_parts:
            return True
        if not path_parts and p_parts:
            # Check if remaining pattern is all star_star
            for i in range(len(p_parts)):
                if p_parts[i][0] != 'star_star':
                    return False
            return True
        if not p_parts and path_parts:
            return False

        # Handle star_star logic which is tricky because it spans segments
        # We need to find the first non-star_star part or end
        # Strategy: iterate through path_parts and p_parts
        
        # Optimization: if p_parts starts with star_star, it can match 0 or more segments
        # Let's try to match the rest of the pattern against the remaining path segments
        
        # Find the first non-star_star in p_parts
        first_real = -1
        for i, p in enumerate(p_parts):
            if p[0] != 'star_star':
                first_real = i
                break
        
        if first_real == -1:
            # All remaining are star_star, match anything
            return True
        
        # Try matching 0, 1, ... segments of path_parts to the initial star_stars
        # The star_stars match everything up to the first non-star_star pattern
        
        # Let's extract the prefix of star_stars
        star_star_count = first_real
        
        # We need to match path_parts[0:k] to the star_star_count (which means 0 or more segments)
        # Actually, multiple ** can match 0 or more each, but effectively they act as one wildcard for segments.
        # But the spec says 'a/**/b' matches 'a/b' (0 segments) and 'a/x/y/b' (2 segments).
        # So we try all possible splits for the segments consumed by the ** sequence.
        
        # However, standard glob ** matches 0 or more segments.
        # If we have multiple **, they are treated as a single wildcard for segments in most implementations,
        # but technically each ** can match 0 or more.
        # Let's simplify: if we have a sequence of ** at the start, it matches 0 or more segments.
        # We need to find a split point in path_parts such that the rest of the path matches the rest of the pattern.
        
        # The "rest of the pattern" starts at index `first_real`.
        # The "rest of the path" starts at index `k`.
        # We try k from 0 to len(path_parts).
        
        for k in range(len(path_parts) + 1):
            # Check if the prefix of path_parts (0 to k) can be consumed by the star_stars
            # Since star_star matches 0 or more segments, any k is valid for the ** part.
            # But we must ensure that the segments between ** are not empty if there are multiple **?
            # Actually, ** matches 0 or more segments. So we just need to check if the rest matches.
            
            # Wait, if we have 'a/**/**/b', it's equivalent to 'a/**/b'.
            # But if we have 'a/**/b/**/c', then first ** matches some segments, then 'b' matches a segment, then second ** matches some.
            
            # Let's implement a recursive check for the segments.
            # We are at path index `k` and pattern index `first_real`.
            if match_segments(path_parts[k:], p_parts[first_real:]):
                return True
        
        return False

    def match_segments(path_list, pattern_list):
        # path_list: list of segments
        # pattern_list: list of pattern parts (can include star_star)
        # This function handles the logic where star_star consumes segments
        
        # If pattern_list is empty, path_list must be empty
        if not pattern_list:
            return len(path_list) == 0
        
        # If path_list is empty, pattern_list must be all star_star
        if not path_list:
            return all(p[0] == 'star_star' for p in pattern_list)
        
        # Find first non-star_star
        first_real = -1
        for i, p in enumerate(pattern_list):
            if p[0] != 'star_star':
                first_real = i
                break
        
        if first_real == -1:
            return True
        
        # Try all possible splits for the initial star_stars
        # The star_stars match 0 or more segments.
        # We try to match pattern_list[first_real:] against path_list[k:]
        # for all k >= 0.
        
        # However, we must be careful about the segments consumed by star_star.
        # The segments consumed by star_star are just ignored.
        # So we just try every possible k.
        
        for k in range(len(path_list) + 1):
            # Check if the prefix path_list[0:k] is valid for the star_stars
            # Since ** matches 0 or more, any k is valid.
            # But we need to ensure that the segments we skip are not conflicting with the next pattern part?
            # No, ** consumes them entirely.
            
            # However, if we have multiple ** in a row, e.g. a/**/**/b
            # The first ** can match 0 segments, then the second ** matches 0 segments, then 'b' matches.
            # Or first ** matches 1, second matches 0.
            # The logic of trying all k for the first block of ** covers this.
            
            # But wait, what if the pattern is a/**/b and path is a/x/b?
            # first_real points to 'b' (literal).
            # We try k=0: match 'b' against 'x' -> fail.
            # k=1: match 'b' against 'b' -> success.
            
            # What if pattern is a/**/**/b and path is a/x/b?
            # first_real points to 'b'.
            # We try k=0: match 'b' against 'x' -> fail.
            # k=1: match 'b' against 'b' -> success.
            # k=2: match 'b' against 'b' (if path was longer) -> success.
            
            # This seems correct.
            
            # One edge case: if the pattern starts with *, it matches 0 chars in a segment.
            # But here we are dealing with segments.
            
            # Is there a constraint that ** cannot match an empty set of segments if followed by a literal?
            # No, 'a/**/b' matches 'a/b'.
            
            # So we just iterate k.
            if match_segments(path_list[k:], pattern_list[first_real:]):
                return True
        
        return False

    # Wait, the logic above for star_star is slightly flawed because it assumes star_star is at the start of the pattern list.
    # If the pattern is 'a/**/b', the parts are ['a', 'star_star', 'b'].
    # When matching 'a/b' (segments ['a', 'b']):
    # match_segments(['a', 'b'], ['a', 'star_star', 'b'])
    # first_real = 1 (star_star).
    # We try k=0: match_segments(['a', 'b'], ['a', 'b']) -> 
    #   first_real = 0 (literal 'a').
    #   Match 'a' against 'a' -> success.
    #   Then match_segments(['b'], ['b']) -> match 'b' against 'b' -> success.
    #   So k=0 works.
    # We try k=1: match_segments(['b'], ['a', 'b']) -> 
    #   first_real = 0. Match 'a' against 'b' -> fail.
    # So it works.
    
    # But what if pattern is 'a/**/b' and path is 'a/x/y/b'?
    # match_segments(['a', 'x', 'y', 'b'], ['a', 'star_star', 'b'])
    # first_real = 1.
    # k=0: match_segments(['a', 'x', 'y', 'b'], ['a', 'b']) -> 
    #   match 'a' vs 'a' -> ok.
    #   match_segments(['x', 'y', 'b'], ['b']) -> 
    #     first_real = 0 (literal 'b').
    #     k=0: 'b' vs 'x' -> fail.
    #     k=1: 'b' vs 'y' -> fail.
    #     k=2: 'b' vs 'b' -> success.
    #     So k=2 works.
    # So the logic holds.
    
    # However, there is a problem with the recursive call in the loop.
    # In the loop, we call match_segments(path_list[k:], pattern_list[first_real:]).
    # But pattern_list[first_real:] starts with a non-star_star (by definition).
    # But wait, what if pattern_list has multiple star_stars at the beginning?
    # e.g. '**/a/**/b'
    # parts = ['star_star', 'star_star', 'a', 'star_star', 'b']
    # first_real = 2 (literal 'a').
    # We try k=0: match_segments(path, ['a', 'star_star', 'b'])
    #   first_real = 0.
    #   Match 'a' against path[0]...
    # This seems to handle multiple ** correctly by skipping them all in one go.
    
    # But wait, the initial logic in `match_path` calls `match_segments` which then loops.
    # The `match_segments` function handles the star_star at the beginning of its input.
    # But what if the pattern is just 'a' and path is 'a'?
    # match_segments(['a'], ['a']) -> first_real=0.
    # Loop k=0: match_segments(['a'], ['a']) -> recursive? No.
    # Wait, the logic inside match_segments:
    # If first_real != -1, we loop k from 0 to len(path_list).
    # Then we call match_segments(path_list[k:], pattern_list[first_real:]).
    # If k=0, we call match_segments(path_list, pattern_list[1:]).
    # This effectively skips the first pattern part if it was star_star?
    # No, first_real is the index of the first NON-star_star.
    # So pattern_list[0:first_real] are all star_star.
    # We are skipping them by jumping to first_real.
    # And we are skipping k segments of the path.
    # This is correct.
    
    # But wait, what if the pattern is just 'a' and path is 'a'?
    # first_real = 0.
    # Loop k=0: match_segments(['a'], ['a']) -> 
    #   Wait, pattern_list[first_real:] is ['a'].
    #   So we call match_segments(['a'], ['a']) again?
    #   No, the loop is:
    #   for k in range(len(path_list) + 1):
    #       if match_segments(path_list[k:], pattern_list[first_real:]):
    #   So if k=0, we call match_segments(['a'], ['a']).
    #   This is the same call. Infinite recursion.
    
    # Ah, the issue is that if the first part is NOT a star_star, we should not loop k.
    # We should match the first part against the first segment.
    # The loop is only for star_star.
    
    # Correction:
    # If the first part is a literal, question, or star, we must match it against the first segment.
    # The star_star is the only one that can skip segments.
    
    # Let's rewrite the matching logic to be more robust.
    
    # Revised strategy:
    # We have a list of path segments and a list of pattern parts.
    # We process the pattern parts one by one.
    # If we encounter a star_star, we try to match the rest of the pattern against the rest of the path
    # for all possible