```python
def match(pattern, path):
    """Return True if the glob pattern matches the whole path."""
    p_segs = pattern.split('/')
    s_segs = path.split('/')

    def match_segment(pat, seg):
        """Match a single pattern segment against a path segment."""
        memo = {}

        def rec(i, j):
            key = (i, j)
            if key in memo:
                return memo[key]
            if i == len(pat):
                res = j == len(seg)
            else:
                c = pat[i]
                if c == '\\':
                    if i + 1 >= len(pat):
                        # Treat trailing backslash as literal
                        res = j < len(seg) and pat[i] == seg[j] and rec(i + 1, j + 1)
                    else:
                        res = j < len(seg) and pat[i + 1] == seg[j] and rec(i + 2, j + 1)
                elif c == '?':
                    res = j < len(seg) and rec(i + 1, j + 1)