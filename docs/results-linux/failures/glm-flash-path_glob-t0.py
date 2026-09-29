def match(pattern, path):
    # Helper to match a pattern segment against a path segment
    def segment_match(p_seg, s_seg):
        i = 0
        j = 0
        p_len = len(p_seg)
        s_len = len(s_seg)

        # Stack to handle recursive backtracking for '*'
        # Each entry is (i, j, star_pos, star_char)
        stack = []

        while j < s_len:
            if i < p_len:
                pc = p_seg[i]
                sc = s_seg[j]

                # Handle escaped characters
                if pc == '\\':
                    if i + 1 < p_len:
                        if pc == s_seg[j]:
                            i += 1
                            j += 1
                        else:
                            return False
                    else:
                        # Trailing backslash matches literal backslash
                        if pc == sc:
                            i += 1
                            j += 1
                        else:
                            return False
                    continue

                # Handle Character Classes
                if pc == '[':
                    # Find the closing bracket
                    close_idx = p_seg.find(']', i + 1)
                    if close_idx == -1:
                        # Unclosed class, treat as literal
                        if pc == sc:
                            i += 1
                            j += 1
                        else:
                            return False
                        continue

                    class_str = p_seg[i + 1:close_idx]
                    negated = False
                    if class_str.startswith('!'):
                        negated = True
                        class_str = class_str[1:]

                    # Parse ranges and single chars
                    in_range = False
                    match = False
                    k = 0
                    while k < len(class_str):
                        c1 = class_str[k]
                        if k + 1 < len(class_str) and class_str[k + 1] == '-':
                            c2 = class_str[k + 2]
                            if c1 <= sc <= c2:
                                match = True
                            k += 2
                        else:
                            if c1 == sc:
                                match = True
                            k += 1

                    if negated:
                        match = not match

                    if match:
                        i += 1
                        j += 1
                    else:
                        return False
                    continue

                # Handle Wildcards
                if pc == '?':
                    i += 1
                    j += 1
                    continue

                if pc == '*':
                    # Save current state for backtracking
                    stack.append((i, j, i, pc))
                    # Consume the character and advance pattern to try matching '*'
                    i += 1
                    continue

                # Literal match
                if pc == sc:
                    i += 1
                    j += 1
                else:
                    return False
            else:
                # Pattern exhausted, but path has chars left.
                # Check if we can use a previous '*' to consume them.
                if stack:
                    i, j, star_pos, star_char = stack.pop()
                    # Advance path char, keep pattern at '*'
                    j += 1
                    # Push back state (pattern stays at star_pos)
                    stack.append((i, j, star_pos, star_char))
                else:
                    return False

        # Path exhausted, check if pattern is also exhausted
        # Handle trailing '*' or '**'
        while i < p_len:
            if p_seg[i] == '*':
                i += 1
            else:
                return False

        return True

    # Split path by '/'
    path_parts = path.split('/')
    # Split pattern by '/'
    pattern_parts = pattern.split('/')

    # Handle leading '**' in pattern
    # If pattern starts with '**', it matches the empty prefix of the path
    # We effectively ignore the first '**' and try to match the rest
    if pattern_parts and pattern_parts[0] == '**':
        pattern_parts = pattern_parts[1:]

    # Check if number of parts matches
    if len(pattern_parts) != len(path_parts):
        return False

    # Match each segment
    for p, s in zip(pattern_parts, path_parts):
        if not segment_match(p, s):
            return False

    return True