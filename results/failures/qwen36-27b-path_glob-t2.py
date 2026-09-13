def match(pattern: str, path: str) -> bool:
    def _match(pattern: str, path: str) -> bool:
        pi = 0
        si = 0
        plen = len(pattern)
        slen = len(path)
        star_pi = -1
        star_si = -1

        while si < slen:
            if pi < plen and pattern[pi] == '\\':
                # Escaped character
                if pi + 1 < plen:
                    if path[si] == pattern[pi + 1]:
                        pi += 2
                        si += 1
                        continue
                    else:
                        # Mismatch after escape
                        if star_pi >= 0:
                            star_si += 1
                            pi = star_pi + 1
                            si = star_si
                            continue
                        else:
                            return False
                else:
                    # Trailing backslash, treat as literal? Or error?
                    # Usually trailing backslash is invalid or matches nothing.
                    # Let's treat it as a mismatch.
                    if star_pi >= 0:
                        star_si += 1
                        pi = star_pi + 1
                        si = star_si
                        continue
                    else:
                        return False
            elif pi < plen and (pattern[pi] == '?' or pattern[pi] == path[si]):
                pi += 1
                si += 1
            elif pi < plen and pattern[pi] == '*':
                # '*' matches zero or more non-'/' characters
                if pi + 1 < plen and pattern[pi + 1] == '*':
                    # '**' matches zero or more whole segments
                    # Find the next '/' after '**'
                    double_star_end = pi + 2
                    if double_star_end < plen and pattern[double_star_end] == '/':
                        double_star_end += 1
                    
                    # Try matching zero, one, ... segments
                    # We need to match the rest of the pattern against the rest of the path
                    # The '**' can consume 0 or more segments.
                    # A segment is a sequence of characters not containing '/'.
                    
                    # Strategy:
                    # If there are no more segments in path, we can match zero segments.
                    # Otherwise, we try to match the rest of the pattern starting from different positions in path.
                    
                    # Save state for backtracking if we treat '**' like '*' but segment-aware?
                    # Actually, '**' is greedy but needs to match whole segments.
                    # Let's use the standard star backtracking logic but adapted for segments.
                    
                    # Record the position of '**'
                    star_pi = pi
                    star_si = si
                    pi = double_star_end
                    
                    # We need to advance pi past '**' and optional '/'
                    # And we need to try matching the rest of the pattern against the rest of the path
                    # The key is that '**' matches across '/'.
                    
                    # Let's continue the loop. If we fail, we backtrack star_si to the next segment start.
                    # But standard star backtracking increments star_si by 1.
                    # For '**', we should jump to the next segment boundary.
                    
                    # However, the standard algorithm works if we handle the backtracking correctly.
                    # When we backtrack from '**', we need to advance star_si to the end of the current segment.
                    
                    # Let's modify the backtracking logic for '**'.
                    # We'll use a flag or check if the star was a double star.
                    pass # Continue to next iteration, backtracking logic will handle it if we adjust it.
                else:
                    # Single '*'
                    star_pi = pi
                    star_si = si
                    pi += 1
                continue
            elif star_pi >= 0:
                # Backtrack
                # If the star was '**', we need to jump to the next segment
                # Check if pattern[star_pi] == '*' and pattern[star_pi+1] == '*'
                is_double_star = (star_pi + 1 < plen and pattern[star_pi + 1] == '*')
                
                if is_double_star:
                    # Advance star_si to the end of the current segment
                    while star_si < slen and path[star_si] != '/':
                        star_si += 1
                    # Now star_si points to '/' or end
                    # If it points to '/', we want to skip the '/' and start matching from the next segment
                    if star_si < slen:
                        star_si += 1
                    
                    # If we've consumed the whole path, this backtrack fails
                    if star_si > slen:
                        return False
                    
                    si = star_si
                    # Reset pi to after the '**' and optional '/'
                    double_star_end = star_pi + 2
                    if double_star_end < plen and pattern[double_star_end] == '/':
                        double_star_end += 1
                    pi = double_star_end
                else:
                    # Single '*' backtracking: advance by one character
                    star_si += 1
                    if star_si > slen:
                        return False
                    si = star_si
                    pi = star_pi + 1
                continue
            else:
                return False

        # Consume remaining '*' in pattern
        while pi < plen and pattern[pi] == '*':
            # Check for '**'
            if pi + 1 < plen and pattern[pi + 1] == '*':
                # '**' at the end matches zero or more segments, which is valid if path is consumed
                # But we need to ensure the rest of the pattern is also consumed
                pi += 2
                if pi < plen and pattern[pi] == '/':
                    # Trailing '/**/'? If path is consumed, this might fail if there's more pattern
                    # Actually if si == slen, we are done with path.
                    # If there is more pattern after '**', it must be empty or handled.
                    # If pattern is 'a/**', and path is 'a', we need to match '' against ''.
                    # The '**' consumes zero segments.
                    pi += 1 # skip '/'
                    continue
                continue
            else:
                pi += 1
        
        return pi == plen

    # Handle character classes manually
    # Pre-process pattern to expand character classes into a format easier for matching?
    # Or handle them in the loop.
    # Let's rewrite _match to handle character classes.

    def _match_with_classes(pattern: str, path: str) -> bool:
        pi = 0
        si = 0
        plen = len(pattern)
        slen = len(path)
        star_pi = -1
        star_si = -1
        is_double_star = False

        while si < slen:
            if pi < plen and pattern[pi] == '\\':
                if pi + 1 < plen:
                    if path[si] == pattern[pi + 1]:
                        pi += 2
                        si += 1
                        continue
                    else:
                        if star_pi >= 0:
                            if is_double_star:
                                while star_si < slen and path[star_si] != '/':
                                    star_si += 1
                                if star_si < slen:
                                    star_si += 1
                                if star_si > slen:
                                    return False
                                si = star_si
                                double_star_end = star_pi + 2
                                if double_star_end < plen and pattern[double_star_end] == '/':
                                    double_star_end += 1
                                pi = double_star_end
                            else:
                                star_si += 1
                                if star_si > slen:
                                    return False
                                si = star_si
                                pi = star_pi + 1
                            continue
                        else:
                            return False
                else:
                    if star_pi >= 0:
                        if is_double_star:
                            while star_si < slen and path[star_si] != '/':
                                star_si += 1
                            if star_si < slen:
                                star_si += 1
                            if star_si > slen:
                                return False
                            si = star_si
                            double_star_end = star_pi + 2
                            if double_star_end < plen and pattern[double_star_end] == '/':
                                double_star_end += 1
                            pi = double_star_end
                        else:
                            star_si += 1
                            if star_si > slen:
                                return False
                            si = star_si
                            pi = star_pi + 1
                        continue
                    else:
                        return False

            # Check for character class
            if pi < plen and pattern[pi] == '[':
                # Find the closing bracket
                class_end = -1
                negate = False
                j = pi + 1
                if j < plen and pattern[j] == '!':
                    negate = True
                    j += 1
                if j < plen and pattern[j] == ']':
                    j += 1
                while j < plen and pattern[j] != ']':
                    j += 1
                if j >= plen:
                    # No closing bracket, treat '[' as literal
                    if path[si] == '[':
                        pi += 1
                        si += 1
                        continue
                    else:
                        # Mismatch
                        if star_pi >= 0:
                            if is_double_star:
                                while star_si < slen and path[star_si] != '/':
                                    star_si += 1
                                if star_si < slen:
                                    star_si += 1
                                if star_si > slen:
                                    return False
                                si = star_si
                                double_star_end = star_pi + 2
                                if double_star_end < plen and pattern[double_star_end] == '/':
                                    double_star_end += 1
                                pi = double_star_end
                            else:
                                star_si += 1
                                if star_si > slen:
                                    return False
                                si = star_si
                                pi = star_pi + 1
                            continue
                        else:
                            return False
                
                class_str = pattern[pi+1:j]
                char = path[si]
                
                # Check if char is in class
                if char == '/':
                    # Class never matches '/'
                    in_class = False
                else:
                    in_class = False
                    k = 0
                    if negate:
                        k = 1
                    while k < len(class_str):
                        if k + 2 < len(class_str) and class_str[k+1] == '-':
                            # Range
                            start = class_str[k]
                            end = class_str[k+2]
                            if start <= char <= end:
                                in_class = True
                                break
                            k += 3
                        else:
                            if char == class_str[k]:
                                in_class = True
                                break
                            k += 1
                    if negate:
                        in_class = not in_class
                
                pi = j + 1 # Move past ']'
                
                if in_class:
                    si += 1
                    continue
                else:
                    # Mismatch
                    if star_pi >= 0:
                        if is_double_star:
                            while star_si < slen and path[star_si] != '/':
                                star_si += 1
                            if star_si < slen:
                                star_si += 1
                            if star_si > slen:
                                return False
                            si = star_si
                            double_star_end = star_pi + 2
                            if double_star_end < plen and pattern[double_star_end] == '/':
                                double_star_end += 1
                            pi = double_star_end
                        else:
                            star_si += 1
                            if star_si > slen:
                                return False
                            si = star_si
                            pi = star_pi + 1
                        continue
                    else:
                        return False

            elif pi < plen and pattern[pi] == '?':
                # '?' matches any char except '/'
                if path[si] != '/':
                    pi += 1
                    si += 1
                    continue
                else:
                    # Mismatch
                    if star_pi >= 0:
                        if is_double_star:
                            while star_si < slen and path[star_si] != '/':
                                star_si += 1
                            if star_si < slen:
                                star_si += 1
                            if star_si > slen:
                                return False
                            si = star_si
                            double_star_end = star_pi + 2
                            if double_star_end < plen and pattern[double_star_end] == '/':
                                double_star_end += 1
                            pi = double_star_end
                        else:
                            star_si += 1
                            if star_si > slen:
                                return False
                            si = star_si
                            pi = star_pi + 1
                        continue
                    else:
                        return False

            elif pi < plen and pattern[pi] == '*':
                if pi + 1 < plen and pattern[pi + 1] == '*':
                    # '**'
                    star_pi = pi
                    star_si = si
                    is_double_star = True
                    pi += 2
                    if pi < plen and pattern[pi] == '/':
                        pi += 1
                    continue
                else:
                    # '*'
                    star_pi = pi
                    star_si = si
                    is_double_star = False
                    pi += 1
                    continue

            elif pi < plen and pattern[pi] == path[si]:
                pi += 1
                si += 1
                continue
            else:
                # Mismatch
                if star_pi >= 0:
                    if is_double_star:
                        while star_si < slen and path[star_si] != '/':
                            star_si += 1
                        if star_si < slen:
                            star_si += 1
                        if star_si > slen:
                            return False
                        si = star_si
                        double_star_end = star_pi + 2
                        if double_star_end < plen and pattern[double_star_end] == '/':
                            double_star_end += 1
                        pi = double_star_end
                    else:
                        star_si += 1
                        if star_si > slen:
                            return False
                        si = star_si
                        pi = star_pi + 1
                    continue
                else:
                    return False

        # Consume remaining '*' in pattern
        while pi < plen:
            if pattern[pi] == '*':
                if pi + 1 < plen and pattern[pi + 1] == '*':
                    pi += 2
                    if pi < plen and pattern[pi] == '/':
                        pi += 1
                    continue
                else:
                    pi += 1
                    continue
            else:
                return False
        
        return pi == plen

    return _match_with_classes(pattern, path)