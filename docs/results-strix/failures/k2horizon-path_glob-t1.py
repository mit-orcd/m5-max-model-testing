def match(pattern, path):
    return _match_segment(pattern.split('/'), path.split('/'), 0, 0)

def _match_segment(pat, path, i, j):
    # pat[i:], path[j:]
    while i < len(pat):
        p = pat[i]
        if p == '**':
            # '**' matches zero or more whole segments
            # Try matching '**' against zero segments, or consume one path segment
            if j < len(path):
                # consume one segment, stay on same '**'
                if _match_segment(pat, path, i, j + 1):
                    return True
            # try next pattern segment (zero segments consumed by '**')
            return _match_segment(pat, path, i + 1, j)
        elif p == '*':
            # '*' matches zero or more chars within this segment
            if _match_star(pat, path, i, j):
                return True
            return False
        elif p == '?' or p == '[':
            # literal segment matching
            if j >= len(path):
                return False
            if not _match_char_class(pat[i], path[j], i):
                return False
            # move to next pattern segment
            return _match_segment(pat, path, i + 1, j + 1)
        else:
            # literal character in pattern segment
            if j >= len(path):
                return False
            if p != path[j]:
                return False
            return _match_segment(pat, path, i + 1, j + 1)
    # pattern exhausted
    return j >= len(path)

def _match_char_class(p, s, i):
    # p is a single pattern segment token at index i (may be '?' or '[')
    if p == '?':
        return len(s) == 1
    # '[' character class
    # parse class starting at i
    j = i + 1
    neg = False
    if j < len(p) and p[j] == '!':
        neg = True
        j += 1
    matched = False
    start = j
    while j < len(p):
        if p[j] == ']':
            break
        j += 1
    # class body is p[start:j]
    if j == len(p):
        # no closing bracket, treat '[' literally
        return s == '['
    body = p[start:j]
    # check if s[0] matches any char in body
    k = 0
    while k < len(body):
        if k + 2 < len(body) and body[k+1] == '-':
            # range body[k] to body[k+2]
            lo, hi = body[k], body[k+2]
            if lo <= s[0] <= hi:
                matched = True
                break
            k += 3
        else:
            if body[k] == s[0]:
                matched = True
                break
            k += 1
    if neg:
        matched = not matched
    return matched

def _match_star(pat, path, i, j):
    # p is '*', matches within segment
    if j >= len(path):
        return i + 1 == len(pat) or (i + 1 < len(pat) and pat[i+1] == '**')
    # try matching '*' against zero or more chars of path[j]
    # We need to match the rest of pattern after '*' against remaining chars
    # '*' can consume k chars (0..len(path[j])-1) then next pattern token must match
    # but '*' is within a segment, so we match it against chars of path[j]
    # We need to find a split: '*' consumes some chars, then next pattern segment token matches rest
    # Since '*' is alone in its segment? Actually '*' can be part of segment like 'a*b'
    # Simpler: handle '*' as a segment-level wildcard that can match part of path[j]
    # We'll match '*' against a prefix of path[j], then continue pattern from i+1 against remaining
    # But path[j] may have more chars after '*' consumed. We need to match remaining pattern tokens
    # against the rest of path[j] (same segment) or subsequent segments.
    # Implement: '*' matches k chars (0..len(path[j])), then pattern[i+1:] must match
    # path[j][k:] as a continuation (same segment) plus rest.
    # We'll recursively try all k.
    seg = path[j]
    for k in range(len(seg) + 1):
        if _match_rest(pat, path, i + 1, j, k):
            return True
    return False

def _match_rest(pat, path, i, j, k):
    # path[j] has k chars consumed by '*', remaining is path[j][k:]
    # pattern pat[i:] must match remaining
    if k < len(path[j]):
        # still within same segment, pattern must match remaining chars
        # next pattern token must match path[j][k:]
        if i < len(pat):
            p = pat[i]
            if p == '*':
                # another '*' can match rest
                return _match_star(pat, path, i, j)  # not quite
            # literal token match against path[j][k:]
            # This is getting complex; simplify by treating '*' at segment level only
            pass
    # if k == len(path[j]), move to next segment
    if k == len(path[j]):
        return _match_segment(pat, path, i, j + 1)
    return False