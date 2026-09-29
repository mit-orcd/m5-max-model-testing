def match(pattern, path):
    """
    Return True if a glob pattern matches a whole path.
    Paths are separated by '/'.
    '?' matches exactly one character but never '/'.
    '*' matches zero or more characters but never '/'.
    A path segment that is exactly '**' matches zero or more whole segments.
    Square brackets are a character class matching one character:
      '[abc]', ranges like '[a-z]', and '[!...]' for negation.
      A class never matches '/'.
    A backslash escapes the next character so it is treated literally.
    The whole path must match, not a prefix.
    """
    import re

    # Parse the pattern into a regex.
    # We need to handle:
    # - '**' as a whole segment: matches zero or more whole segments
    # - '*' : zero or more non-'/' chars
    # - '?' : one non-'/' char
    # - [...] : character class, one char, not '/'
    # - backslash escapes
    # - literal chars

    def parse_pattern(pattern):
        """Convert glob pattern to a regex string."""
        regex_parts = []
        i = 0
        n = len(pattern)
        while i < n:
            c = pattern[i]
            if c == '\\':
                # Escape: next char is literal
                if i + 1 < n:
                    nxt = pattern[i + 1]
                    regex_parts.append(re.escape(nxt))
                    i += 2
                else:
                    regex_parts.append(re.escape(c))
                    i += 1
            elif c == '*':
                # Check if this is a '**' segment
                # '**' must be a whole segment: preceded by start or '/' and followed by end or '/'
                # Actually, the problem says "A path segment that is exactly '**'".
                # So we need to detect when the pattern has a segment that is exactly '**'.
                # Let's handle this differently: first split into segments, then build regex.
                # Actually, it's easier to process segment by segment.
                # Let me restructure: split pattern and path by '/' but handle '**' specially.
                # I'll use a different approach: convert pattern to regex by processing it carefully.
                # For now, let's handle '*' as matching zero or more non-slash chars.
                # But '**' as a whole segment is special.
                # Let me restart with a segment-based approach.
                break
            elif c == '?':
                regex_parts.append('[^/]')
                i += 1
            elif c == '[':
                # Parse character class
                j = i + 1
                negated = False
                if j < n and pattern[j] == '!':
                    negated = True
                    j += 1
                # Find closing bracket
                if j >= n:
                    # No closing bracket, treat '[' as literal
                    regex_parts.append(re.escape(c))
                    i += 1
                    continue
                # Check if ']' is the first char in the class (literal ']')
                class_chars = []
                if j < n and pattern[j] == ']':
                    class_chars.append(re.escape(']'))
                    j += 1
                while j < n and pattern[j] != ']':
                    if j + 2 < n and pattern[j + 1] == '-':
                        # Range
                        start_c = pattern[j]
                        end_c = pattern[j + 2]
                        class_chars.append(f'{re.escape(start_c)}-{re.escape(end_c)}')
                        j += 3
                    else:
                        class_chars.append(re.escape(pattern[j]))
                        j += 1
                if j >= n:
                    # No closing bracket
                    regex_parts.append(re.escape(c))
                    i += 1
                    continue
                # j is at ']'
                if negated:
                    # Negated class: match any char not in the class, but not '/'
                    # We need to be careful: [!...] should not match '/'
                    # Build the inner class
                    inner = ''.join(class_chars)
                    # The negated class in regex: [^...] but we also need to exclude '/'
                    # So: [^/inner] -- wait, that's not right.
                    # [!abc] means match any char NOT in {a,b,c}. But it should not match '/'.
                    # So the regex should be: [^/abc] -- no, that's not right either.
                    # Actually, the class [!abc] matches any single character that is not a, b, or c.
                    # And per the problem, a class never matches '/'.
                    # So we need: any char that is not in the class AND is not '/'.
                    # In regex: [^/abc] would mean not /, not a, not b, not c. But we want: not (a or b or c) AND not /.
                    # That's the same as: [^/abc] if the class contains a,b,c.
                    # Wait: [^/abc] means the char must not be /, a, b, or c. That's correct.
                    # But what if the class has a range? Like [!a-z]?
                    # Then inner would be 'a-z', and we'd want [^/a-z].
                    # But [^/a-z] in regex means: not /, and not in range a-z. That's correct.
                    # However, we need to make sure the inner part is properly escaped.
                    # Let's build it as: [^/ + inner]
                    # But wait, inner might contain characters that need to be in a char class context.
                    # Actually, since we're building a char class, we just concatenate.
                    # But there's a subtlety: if inner contains ']', it would break the class.
                    # But we already handled the case where ']' is the first char.
                    # For other cases, ']' should not appear in the class body (we stop at first ']').
                    # So it should be fine.
                    regex_parts.append(f'[^/{inner}]')
                else:
                    inner = ''.join(class_chars)
                    regex_parts.append(f'[{inner}]')
                i = j + 1
            else:
                regex_parts.append(re.escape(c))
                i += 1

        # This approach doesn't handle '**' properly. Let me use a segment-based approach instead.
        return None

    # Segment-based approach
    # Split pattern into segments by '/'
    # But we need to handle '**' as a whole segment.

    # First, let's parse the pattern into a list of segment patterns.
    # A segment pattern is either '**' or a regular glob pattern for a single segment.

    def parse_segments(pattern):
        """Split pattern into segments, handling escaping."""
        segments = []
        current = []
        i = 0
        n = len(pattern)
        while i < n:
            c = pattern[i]
            if c == '\\':
                if i + 1 < n:
                    current.append(pattern[i + 1])
                    i += 2
                else:
                    current.append(c)
                    i += 1
            elif c == '/':
                segments.append(''.join(current))
                current = []
                i += 1
            else:
                current.append(c)
                i += 1
        segments.append(''.join(current))
        return segments

    def segment_to_regex(seg):
        """Convert a single segment's glob pattern to a regex string."""
        regex_parts = []
        i = 0
        n = len(seg)
        while i < n:
            c = seg[i]
            if c == '\\':
                if i + 1 < n:
                    nxt = seg[i + 1]
                    regex_parts.append(re.escape(nxt))
                    i += 2
                else:
                    regex_parts.append(re.escape(c))
                    i += 1
            elif c == '*':
                # In a single segment, '*' matches zero or more non-'/' chars
                regex_parts.append('[^/]*')
                i += 1
            elif c == '?':
                regex_parts.append('[^/]')
                i += 1
            elif c == '[':
                j = i + 1
                negated = False
                if j < n and seg[j] == '!':
                    negated = True
                    j += 1
                if j >= n:
                    regex_parts.append(re.escape(c))
                    i += 1
                    continue
                class_chars = []
                if j < n and seg[j] == ']':
                    class_chars.append(re.escape(']'))
                    j += 1
                while j < n and seg[j] != ']':
                    if j + 2 < n and seg[j + 1] == '-':
                        start_c = seg[j]
                        end_c = seg[j + 2]
                        class_chars.append(f'{re.escape(start_c)}-{re.escape(end_c)}')
                        j += 3
                    else:
                        class_parts = []
                        class_parts.append(re.escape(seg[j]))
                        class_chars.append(class_parts[0])
                        j += 1
                if j >= n:
                    regex_parts.append(re.escape(c))
                    i += 1
                    continue
                inner = ''.join(class_chars)
                if negated:
                    regex_parts.append(f'[^/{inner}]')
                else:
                    regex_parts.append(f'[{inner}]')
                i = j + 1
            else:
                regex_parts.append(re.escape(c))
                i += 1
        return ''.join(regex_parts)

    pattern_segments = parse_segments(pattern)
    path_segments = path.split('/')

    # Now we need to match pattern_segments against path_segments.
    # '**' matches zero or more whole segments.
    # Other segments must match exactly one path segment (via regex).

    # Use dynamic programming.
    # dp[i][j] = True if pattern_segments[i:] matches path_segments[j:]
    m = len(pattern_segments)
    n = len(path_segments)

    # dp is a 2D list of booleans
    dp = [[False] * (n + 1) for _ in range(m + 1)]
    dp[m][n] = True

    # Precompute regex for non-'**' segments
    seg_regex = [None] * m
    for i in range(m):
        if pattern_segments[i] == '**':
            seg_regex[i] = None
        else:
            seg_regex[i] = re.compile('^' + segment_to_regex(pattern_segments[i]) + '$')

    # Fill dp table
    for i in range(m - 1, -1, -1):
        for j in range(n, -1, -1):
            if pattern_segments[i] == '**':
                # '**' can match zero segments (skip it) or one or more segments (match current and continue)
                # Zero segments: dp[i+1][j]
                # One or more: dp[i][j+1] (match one segment and continue with '**')
                # But we need to be careful: '**' matches zero or more, so:
                # dp[i][j] = dp[i+1][j] or (j < n and dp[i][j+1])
                if dp[i + 1][j]:
                    dp[i][j] = True
                elif j < n and dp[i][j + 1]:
                    dp[i][j] = True
            else:
                # Must match exactly one segment
                if j < n:
                    if seg_regex[i].match(path_segments[j]):
                        dp[i][j] = dp[i + 1][j + 1]

    return dp[0][0]