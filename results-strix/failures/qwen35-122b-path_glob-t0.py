def match(pattern, path):
    # Tokenize pattern into a list of tokens
    tokens = []
    i = 0
    n = len(pattern)
    while i < n:
        c = pattern[i]
        if c == '\\':
            if i + 1 < n:
                tokens.append(('LIT', pattern[i+1]))
                i += 2
            else:
                tokens.append(('LIT', '\\'))
                i += 1
        elif c == '?':
            tokens.append(('QMARK',))
            i += 1
        elif c == '*':
            if i + 1 < n and pattern[i+1] == '*':
                tokens.append(('STAR2',))
                i += 2
            else:
                tokens.append(('STAR',))
                i += 1
        elif c == '[':
            j = i + 1
            neg = False
            if j < n and pattern[j] == '!':
                neg = True
                j += 1
            chars = set()
            while j < n and pattern[j] != ']':
                if pattern[j] == '\\' and j + 1 < n:
                    chars.add(pattern[j+1])
                    j += 2
                elif j + 2 < n and pattern[j+1] == '-' and pattern[j+2] != ']':
                    start, end = pattern[j], pattern[j+2]
                    for k in range(ord(start), ord(end) + 1):
                        chars.add(chr(k))
                    j += 3
                else:
                    chars.add(pattern[j])
                    j += 1
            if j < n:
                j += 1
            tokens.append(('CLASS', chars, neg))
            i = j
        else:
            tokens.append(('LIT', c))
            i += 1

    # Split path into segments
    path_segments = path.split('/') if path else []
    if path == '':
        path_segments = []
    elif path == '/':
        path_segments = ['']
    else:
        path_segments = path.split('/')

    # DP state: (token_index, segment_index, char_index_in_segment)
    # But since ** can skip segments, we need to handle segment boundaries carefully.
    # We'll use recursion with memoization.
    from functools import lru_cache

    @lru_cache(maxsize=None)
    def dp(t_idx, s_idx, c_idx):
        # t_idx: current token index
        # s_idx: current path segment index
        # c_idx: current character index within the current segment

        # If we've consumed all tokens
        if t_idx == len(tokens):
            # Must have consumed all path segments and be at start of next segment (or end)
            if s_idx == len(path_segments) and c_idx == 0:
                return True
            # If we are in the middle of a segment, it's a mismatch
            if c_idx > 0:
                return False
            # If we are at start of a segment but there are more segments, mismatch
            if s_idx < len(path_segments):
                return False
            return True

        token = tokens[t_idx]
        kind = token[0]

        if kind == 'LIT':
            char = token[1]
            if s_idx >= len(path_segments):
                return False
            seg = path_segments[s_idx]
            if c_idx >= len(seg):
                return False
            if seg[c_idx] == char:
                return dp(t_idx + 1, s_idx, c_idx + 1)
            return False

        elif kind == 'QMARK':
            if s_idx >= len(path_segments):
                return False
            seg = path_segments[s_idx]
            if c_idx >= len(seg):
                return False
            if seg[c_idx] == '/':
                return False
            return dp(t_idx + 1, s_idx, c_idx + 1)

        elif kind == 'STAR':
            # Match zero or more non-slash characters in current segment
            seg = path_segments[s_idx] if s_idx < len(path_segments) else ""
            # Option 1: match zero characters
            res = dp(t_idx + 1, s_idx, c_idx)
            if res:
                return True
            # Option 2: match one or more characters
            while c_idx < len(seg) and seg[c_idx] != '/':
                if dp(t_idx + 1, s_idx, c_idx + 1):
                    return True
                c_idx += 1
            return False

        elif kind == 'STAR2':
            # Match zero or more whole segments
            # Option 1: match zero segments (skip this token, move to next token, stay at current segment/char)
            if dp(t_idx + 1, s_idx, c_idx):
                return True
            # Option 2: match one or more segments
            # We can consume current segment (if any chars left) and move to next segment
            # But ** matches whole segments, so we must consume entire current segment if we start consuming
            # Actually, ** can match part of a segment? No, "matches zero or more whole segments"
            # So if we are in the middle of a segment, we cannot start matching ** unless we finish the current segment first?
            # Wait, the pattern 'a/**/b' matches 'a/x/y/b'. The ** matches 'x/y'.
            # If we are at 'a/x/y/b', and pattern is 'a/**/b', after matching 'a/', we are at segment 'x'.
            # ** matches 'x', then '/', then 'y', then '/', then 'b' matches 'b'.
            # But what if we are in the middle of a segment? e.g. pattern 'a/**/b' and path 'a/xy/b'?
            # ** should match 'xy' as one segment? Yes, because ** matches whole segments.
            # So if we are at c_idx > 0 in a segment, we cannot start matching ** until we finish the segment?
            # Actually, the problem says: "A path segment that is exactly '**' matches zero or more whole segments"
            # This implies ** is a segment-level wildcard. So it can only match at segment boundaries.
            # Therefore, if c_idx > 0, we cannot use ** to match the rest of the current segment.
            # We must first finish the current segment with other tokens? But ** is the only token that can skip segments.
            # So if c_idx > 0, we cannot match ** unless we have already finished the current segment?
            # Actually, the pattern 'a/**/b' on path 'a/xy/b':
            # After 'a/', we are at segment 'xy', c_idx=0. ** matches 'xy' (one segment), then we are at segment 'b', c_idx=0.
            # Then 'b' matches 'b'.
            # What if pattern is 'a/**/b' and path is 'a/x/y/b'? ** matches 'x' and 'y' (two segments).
            # So ** must be applied at segment boundaries.
            # Therefore, if c_idx > 0, we cannot start matching ** until we finish the current segment?
            # But how do we finish the current segment? We need other tokens to match the rest of the segment.
            # However, ** is the only token that can skip segments. So if we are in the middle of a segment, we cannot use ** to skip the rest of the segment.
            # This implies that ** can only be used when c_idx == 0.
            # So if c_idx > 0, we cannot match ** at all? That seems wrong.
            # Let me re-read: "A path segment that is exactly '**' matches zero or more whole segments"
            # This means the token ** in the pattern corresponds to zero or more entire path segments.
            # So if the pattern has ** and the path has segments, ** can match 0, 1, 2, ... segments.
            # But it must match entire segments. So if we are in the middle of a segment (c_idx > 0), we cannot start matching ** until we finish the current segment.
            # However, the current segment might be partially matched by previous tokens? No, because we are at the token **.
            # So if c_idx > 0, we cannot use ** to match the rest of the current segment. We must have already matched the entire current segment with previous tokens?
            # Actually, the pattern is processed token by token. If we are at ** and c_idx > 0, it means we are in the middle of a segment that hasn't been fully matched yet.
            # But ** is supposed to match whole segments. So it cannot match part of a segment.
            # Therefore, if c_idx > 0, we cannot match ** at all? That would mean ** can only be used at segment boundaries.
            # But what if the pattern is 'a/**/b' and the path is 'a/xy/b'? After matching 'a/', we are at segment 'xy', c_idx=0. Then ** matches 'xy' (one segment), then we are at segment 'b', c_idx=0. Then 'b' matches 'b'.
            # What if the pattern is 'a/**/b' and the path is 'a/x/y/b'? After 'a/', we are at segment 'x', c_idx=0. ** matches 'x' (one segment), then we are at segment 'y', c_idx=0. Then ** is done? No, ** is one token that matches multiple segments.
            # Actually, ** is one token that can match multiple segments. So after matching 'x', we are still at the ** token? No, ** is consumed once it matches zero or more segments.
            # So ** matches a sequence of segments. After matching, we move to the next token and the next segment after the matched ones.
            # Therefore, if we are at ** and c_idx > 0, we cannot match ** because we are in the middle of a segment. We must have already matched the entire current segment with previous tokens? But we are at **, so previous tokens have been consumed.
            # This implies that ** can only be used when c_idx == 0.
            # So if c_idx > 0, we return False for **? That seems correct.
            if c_idx > 0:
                return False
            # Option 1: match zero segments
            if dp(t_idx + 1, s_idx, c_idx):
                return True
            # Option 2: match one or more segments
            # We can match the current segment (if any) and then move to next segments
            # But ** matches whole segments, so we must consume entire current segment
            # However, we are at c_idx=0, so we can consume the entire current segment
            # Then we can consume more segments
            # We try matching 1 segment, 2 segments, etc.
            # But note: we can also match zero segments (already tried)
            # So we try matching k segments for k>=1
            # We start by matching the current segment (if exists)
            if s_idx < len(path_segments):
                # Match current segment
                if dp(t_idx + 1, s_idx + 1, 0):
                    return True
                # Match current segment and more
                # We can recursively try matching more segments
                # But we can also use the same ** token to match more segments? No, ** is consumed after matching zero or more segments.
                # So we need to try matching 1 segment, then 2, etc.
                # We can do this by iterating
                current_s = s_idx
                while current_s < len(path_segments):
                    # Match segments from s_idx to current_s (inclusive)
                    # Then move to next token and next segment
                    if dp(t_idx + 1, current_s + 1, 0):
                        return True
                    current_s += 1
            return False

        elif kind == 'CLASS':
            chars, neg = token[1], token[2]
            if s_idx >= len(path_segments):
                return False
            seg = path_segments[s_idx]
            if c_idx >= len(seg):
                return False
            char = seg[c_idx]
            if char == '/':
                return False
            in_class = char in chars
            if neg:
                in_class = not in_class
            if in_class:
                return dp(t_idx + 1, s_idx, c_idx + 1)
            return False

        return False

    return dp(0, 0, 0)