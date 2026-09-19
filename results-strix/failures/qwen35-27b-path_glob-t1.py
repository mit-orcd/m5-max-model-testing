```python
def match(pattern, path):
    def compile_pattern(p):
        tokens = []
        i = 0
        n = len(p)
        while i < n:
            ch = p[i]
            if ch == '\\':
                if i + 1 < n:
                    tokens.append(('lit', p[i + 1]))
                    i += 2
                else:
                    tokens.append(('lit', '\\'))
                    i += 1
            elif ch == '?':
                tokens.append(('q', None))
                i += 1
            elif ch == '*':
                if i + 1 < n and p[i + 1] == '*':
                    tokens.append(('star2', None))
                    i += 2
                else:
                    tokens.append(('star', None))
                    i += 1
            elif ch == '[':
                j = i + 1
                neg = False
                if j < n and p[j] == '!':
                    neg = True
                    j += 1
                elif j < n and p[j] == '^':
                    neg = True
                    j += 1
                chars = set()
                if j < n and p[j] == ']':
                    chars.add(']')
                    j += 1
                while j < n and p[j] != ']':
                    if p[j] == '\\' and j + 1 < n:
                        chars.add(p[j + 1])
                        j += 2
                    elif j + 2 < n and p[j + 1] == '-' and p[j + 2] != ']':
                        chars.add(range(ord(p[j]), ord(p[j + 2]) + 1))
                        j += 3
                    else:
                        chars.add(p[j])
                        j += 1
                if j >= n:
                    # Unclosed bracket, treat as literal
                    tokens.append(('lit', '['))
                    i += 1
                    continue
                tokens.append(('class', chars, neg))
                i = j + 1
            elif ch == '/':
                tokens.append(('sep', None))
                i += 1
            else:
                tokens.append(('lit', ch))
                i += 1
        tokens.append(('end', None))
        return tokens

    def match_segment(tokens, t_idx, path, p_idx):
        # Match a single segment (no slashes) against pattern tokens until sep or end
        while t_idx < len(tokens) and tokens[t_idx][0] != 'sep' and tokens[t_idx][0] != 'end' and tokens[t_idx][0] != 'star2':
            tok_type = tokens[t_idx][0]
            if p_idx >= len(path):
                return False
            ch = path[p_idx]
            if tok_type == 'lit':
                if ch != tokens[t_idx][1]:
                    return False
            elif tok_type == 'q':
                if ch == '/':
                    return False
            elif tok_type == 'star':
                # '*' within a segment matches any chars except '/'
                # handled by recursive consumption
                return match_star_segment(tokens, t_idx + 1, path, p_idx)
            elif tok_type == 'class':
                if ch == '/':
                    return False
                char_set = tokens[t_idx][1]
                neg = tokens[t_idx][2]
                matched = False
                for c in char_set:
                    if isinstance(c, range):
                        if c.start <= ord(ch) <= c.stop:
                            matched = True
                            break
                    elif ch == c:
                        matched = True
                        break
                if neg:
                    matched = not matched
                if not matched:
                    return False
            p_idx += 1
            t_idx += 1
        return t_idx, p_idx

    def match_star_segment(tokens, t_idx, path, p_idx):
        # '*' matches zero or more non-slash chars
        # Try matching zero, then one, then two...
        # But since we need whole segment match, we can't just consume arbitrarily
        # Actually, '*' inside a segment is like .* but stopping at end of segment or next special
        # We'll try all possible lengths
        max_len = len(path) - p_idx
        for k in range(max_len + 1):
            # Check if next k chars are valid (no slash)
            valid = True
            for j in range(k):
                if path[p_idx + j] == '/':
                    valid = False
                    break
            if not valid:
                break
            # Now try to match rest of pattern from t_idx starting at p_idx + k
            res = match_segment(tokens, t_idx, path, p_idx + k)
            if res:
                return True
        return False

    def match_path(tokens, t_idx, path, p_idx):
        if t_idx >= len(tokens) or p_idx >= len(path):
            return tokens[t_idx][0] == 'end' and p_idx == len(path)

        tok = tokens[t_idx]
        tok_type = tok[0]

        if tok_type == 'end':
            return p_idx == len(path)

        if tok_type == 'sep':
            if p_idx >= len(path) or path[p_idx] != '/':
                return False
            return match_path(tokens, t_idx + 1, path, p_idx + 1)

        if tok_type == 'star2':
            # ** matches zero or more whole segments
            # Try matching zero segments, then one, then two...
            # Zero segments: just skip ** and continue
            if match_path(tokens, t_idx + 1, path, p_idx):
                return True
            # Match one or more segments
            # Find next slash or end
            start = p_idx
            while p_idx < len(path) and path[p_idx] != '/':
                p_idx += 1
            # Now p_idx is at end of segment or at slash
            # Try to match rest of pattern after consuming this segment
            # But ** can consume multiple segments, so we need to try recursively
            # Actually, standard approach: try to match ** as consuming 0, then 1 segment, then 2, etc.
            # But we can optimize: after matching one segment, the rest of the path is still available for ** to match more
            # So we can just try to match the rest of the pattern after this segment, and if it fails, we continue
            # However, since ** can match any number of segments, we can just try to match the rest of the pattern
            # starting from the next segment boundary
            # But we must ensure that we don't consume the last segment if the pattern expects more
            # Actually, the standard way is to try to match the rest of the pattern from the current position
            # and if it fails, try to consume one more segment and try again
            # But this can be inefficient. Instead, we can use recursion with memoization, but for simplicity, we'll do a loop
            # However, note that the pattern after ** might have more ** or other patterns
            # So we try to match the rest of the pattern from the current position (after consuming one segment) and if it fails, we try consuming more
            # But actually, we can just try to match the rest of the pattern from the current position and if it fails, we try to skip one segment and try again
            # But this is complex. Instead, we can use a recursive approach that tries to match the rest of the pattern from the current position
            # and if it fails, we try to skip one segment and try again until we run out of segments
            # However, note that the pattern after ** might start with a segment that matches the current segment, so we can't just skip
            # The standard solution is to try to match the rest of the pattern from the current position (without consuming any segment) and if it fails,
            # then try to consume one segment and try again, and so on until we run out of segments
            # But note: we already tried zero segments above (t_idx+1, p_idx). Now we try consuming one segment and then recursively call match_path
            # But the recursive call will again try zero segments for the next ** if there is one, so it's safe.
            # However, we must be careful: we are at the beginning of a segment. We consume one segment (until next slash or end)