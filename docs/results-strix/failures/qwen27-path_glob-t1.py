def match(pattern, path):
    # Tokenize the pattern into segments split by '/'
    # Handle escapes and character classes within segments
    # Use recursive matching with memoization

    # First, parse the pattern into a list of segment matchers
    # Each segment matcher is a function that checks if it matches a single path segment

    def parse_pattern(pattern):
        """Parse pattern into list of segment matchers."""
        segments = []
        i = 0
        n = len(pattern)
        current = []

        while i < n:
            c = pattern[i]
            if c == '/':
                segments.append(current)
                current = []
                i += 1
            elif c == '\\':
                if i + 1 < n:
                    current.append(('literal', pattern[i + 1]))
                    i += 2
                else:
                    current.append(('literal', '\\'))
                    i += 1
            elif c == '?':
                current.append(('any_char',))
                i += 1
            elif c == '*':
                current.append(('any_seq',))
                i += 1
            elif c == '[':
                # Parse character class
                i += 1
                negate = False
                if i < n and pattern[i] == '!':
                    negate = True
                    i += 1
                chars = []
                ranges = []
                while i < n and pattern[i] != ']':
                    c2 = pattern[i]
                    if c2 == '\\':
                        if i + 1 < n:
                            chars.append(pattern[i + 1])
                            i += 2
                            continue
                        else:
                            chars.append('\\')
                            i += 1
                            continue
                    elif c2 == '-' and i + 2 < n and pattern[i + 2] != ']':
                        # Check if this is a range
                        start = c2
                        end = pattern[i + 2]
                        ranges.append((start, end))
                        i += 3
                        continue
                    else:
                        chars.append(c2)
                        i += 1
                        continue
                if i < n and pattern[i] == ']':
                    i += 1  # skip ]
                current.append(('char_class', negate, chars, ranges))
            else:
                current.append(('literal', c))
                i += 1

        segments.append(current)
        return segments

    def make_segment_matcher(seg_tokens):
        """Create a function that matches a single segment against the tokens."""
        def match_segment(segment):
            # Use backtracking to match segment against tokens
            return _match_tokens(seg_tokens, segment, 0, 0)
        return match_segment

    def _match_tokens(tokens, segment, ti, si):
        """Match tokens against segment using backtracking."""
        if ti == len(tokens):
            return si == len(segment)
        if si == len(segment):
            # Only zero-width matches are possible; but our tokens all consume at least one char
            # except... actually all tokens consume at least one character
            return False

        token = tokens[ti]
        kind = token[0]

        if kind == 'literal':
            if segment[si] == token[1]:
                return _match_tokens(tokens, segment, ti + 1, si + 1)
            else:
                return False
        elif kind == 'any_char':
            # matches any char except '/' (but segment never contains '/')
            return _match_tokens(tokens, segment, ti + 1, si + 1)
        elif kind == 'any_seq':
            # matches zero or more characters
            for end in range(si, len(segment) + 1):
                if _match_tokens(tokens, segment, ti + 1, end):
                    return True
            return False
        elif kind == 'char_class':
            _, negate, chars, ranges = token
            ch = segment[si]
            matched = False
            if ch in chars:
                matched = True
            else:
                for (start, end) in ranges:
                    if start <= ch <= end:
                        matched = True
                        break
            if negate:
                matched = not matched
            if matched:
                return _match_tokens(tokens, segment, ti + 1, si + 1)
            else:
                return False
        return False

    def is_double_star(seg_tokens):
        """Check if a segment is exactly '**'."""
        if len(seg_tokens) != 2:
            return False
        return seg_tokens[0] == ('any_seq',) and seg_tokens[1] == ('any_seq',)

    pattern_segments = parse_pattern(pattern)
    path_segments = path.split('/')

    # Use recursive matching with memoization on (pi, si) where pi is pattern segment index, si is path segment index
    from functools import lru_cache

    @lru_cache(maxsize=None)
    def rec(pi, si):
        if pi == len(pattern_segments):
            return si == len(path_segments)
        if si == len(path_segments):
            # No more path segments; remaining pattern segments must all be '**'
            for k in range(pi, len(pattern_segments)):
                if not is_double_star(pattern_segments[k]):
                    return False
            return True

        pat_seg = pattern_segments[pi]
        path_seg = path_segments[si]

        if is_double_star(pat_seg):
            # '**' matches zero or more whole segments
            # Option 1: match zero segments (skip '**')
            if rec(pi + 1, si):
                return True
            # Option 2: match one or more segments
            for k in range(si + 1, len(path_segments) + 1):
                if rec(pi + 1, k):
                    return True
            return False
        else:
            # Normal segment: must match exactly one path segment
            # Parse this segment's tokens and match against path_seg
            if _match_tokens(pat_seg, path_seg, 0, 0):
                return rec(pi + 1, si + 1)
            return False

    result = rec(0, 0)
    rec.cache_clear()
    return result