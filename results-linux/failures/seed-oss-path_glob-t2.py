def unescape(s):
    res = []
    i = 0
    n = len(s)
    while i < n:
        if s[i] == '\\':
            if i + 1 < n:
                res.append(s[i+1])
                i += 2
            else:
                res.append('\\')
                i += 1
        else:
            res.append(s[i])
            i += 1
    return ''.join(res)

def split_pattern_segments(pattern):
    segments = []
    current = []
    escape = False
    for c in pattern:
        if escape:
            current.append(c)
            escape = False
        else:
            if c == '\\':
                escape = True
            elif c == '/':
                segments.append(''.join(current))
                current = []
            else:
                current.append(c)
    segments.append(''.join(current))
    return [unescape(seg) for seg in segments]

def parse_character_class(content):
    negated = False
    if content.startswith('!'):
        negated = True
        content = content[1:]
    chars = set()
    i = 0
    n = len(content)
    while i < n:
        if i + 2 < n and content[i+1] == '-':
            start = content[i]
            end = content[i+2]
            if ord(start) <= ord(end):
                for o in range(ord(start), ord(end) + 1):
                    chars.add(chr(o))
            else:
                chars.add(content[i])
                chars.add('-')
                chars.add(content[i+2])
            i += 3
        else:
            chars.add(content[i])
            i += 1
    return negated, chars

def matches_segment(pattern, s):
    len_p = len(pattern)
    len_s = len(s)
    dp = [[False] * (len_s + 1) for _ in range(len_p + 1)]
    dp[len_p][len_s] = True

    for i in range(len_p - 1, -1, -1):
        dp[i][len_s] = pattern[i] == '*' and dp[i+1][len_s]

    for i in range(len_p - 1, -1, -1):
        for j in range(len_s - 1, -1, -1):
            p_char = pattern[i]
            if p_char == '*':
                dp[i][j] = dp[i+1][j] or dp[i][j+1]
            elif p_char == '?':
                dp[i][j] = dp[i+1][j+1]
            elif p_char == '[':
                k = i
                while k < len_p and pattern[k] != ']':
                    k += 1
                if k == len_p:
                    dp[i][j] = False
                    continue
                content = pattern[i+1:k]
                negated, chars = parse_character_class(content)
                current_match = j < len_s and ((not negated and s[j] in chars) or (negated and s[j] not in chars))
                dp[i][j] = current_match and dp[k+1][j+1]
            else:
                dp[i][j] = p_char == s[j] and dp[i+1][j+1]
    return dp[0][0]

def match(pattern, path):
    pattern_segments = split_pattern_segments(pattern)
    path_segments = path.split('/')

    p_idx = s_idx = star_idx = s_star_idx = 0
    len_p = len(pattern_segments)
    len_s = len(path_segments)

    while s_idx < len_s:
        if p_idx < len_p and pattern_segments[p_idx] == '**':
            star_idx, s_star_idx = p_idx, s_idx
            p_idx += 1
        elif star_idx != -1:
            p_idx, s_idx, s_star_idx = star_idx + 1, s_star_idx + 1, s_star_idx + 1
        elif p_idx < len_p:
            if matches_segment(pattern_segments[p_idx], path_segments[s_idx]):
                p_idx, s_idx = p_idx + 1, s_idx + 1
            else:
                return False
        else:
            return False

    while p_idx < len_p and pattern_segments[p_idx] == '**':
        p_idx += 1

    return p_idx == len_p