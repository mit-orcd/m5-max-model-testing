def match(pattern, path):
    def parse_pattern(p):
        tokens = []
        i = 0
        while i < len(p):
            if p[i] == '\\' and i + 1 < len(p):
                tokens.append(('literal', p[i + 1]))
                i += 2
            elif p[i] == '[':
                j = i + 1
                negated = False
                if j < len(p) and p[j] == '!':
                    negated = True
                    j += 1
                chars = []
                while j < len(p) and p[j] != ']':
                    if j + 2 < len(p) and p[j + 1] == '-' and j + 2 < len(p):
                        start, end = p[j], p[j + 2]
                        if start <= end:
                            chars.extend(chr(c) for c in range(ord(start), ord(end) + 1))
                        else:
                            chars.extend(chr(c) for c in range(ord(end), ord(start) + 1))
                        j += 3
                    else:
                        chars.append(p[j])
                        j += 1
                if j < len(p) and p[j] == ']':
                    j += 1
                char_set = set(chars)
                if negated:
                    char_set = char_set - set('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_')
                tokens.append(('char_class', char_set))
                i = j
            elif p[i] == '?':
                tokens.append(('question',))
                i += 1
            elif p[i] == '**':
                tokens.append(('star_star',))
                i += 2
            elif p[i] == '*':
                tokens.append(('star',))
                i += 1
            else:
                tokens.append(('literal', p[i]))
                i += 1
        return tokens

    def match_segment(text, pattern_tokens):
        n = len(text)
        m = len(pattern_tokens)
        dp = [[False] * (m + 1) for _ in range(n + 1)]
        dp[0][0] = True
        for j in range(1, m + 1):
            if pattern_tokens[j - 1][0] == 'star':
                dp[0][j] = dp[0][j - 1]
            else:
                break
        for i in range(1, n + 1):
            for j in range(1, m + 1):
                token = pattern_tokens[j - 1]
                if token[0] == 'literal':
                    dp[i][j] = dp[i - 1][j - 1] and text[i - 1] == token[1]
                elif token[0] == 'question':
                    dp[i][j] = dp[i - 1][j - 1]
                elif token[0] == 'char_class':
                    dp[i][j] = dp[i - 1][j - 1] and text[i - 1] in token[1]
                elif token[0] == 'star':
                    dp[i][j] = dp[i][j - 1] or dp[i - 1][j]
        return dp[n][m]

    path_segments = path.split('/')
    pattern_segments = pattern.split('/')
    
    if len(pattern_segments) != len(path_segments):
        star_star_indices = []
        for i, seg in enumerate(pattern_segments):
            if seg == '**':
                star_star_indices.append(i)
        
        if len(star_star_indices) == 0:
            return False
        
        for num_segments in range(1, len(path_segments) - len(pattern_segments) + 2):
            new_path_segments = []
            for i in range(len(path_segments)):
                if i in star_star_indices and num_segments > 0:
                    for _ in range(min(num_segments, len(path_segments) - len(new_path_segments) - (len(pattern_segments) - len(star_star_indices)))):
                        new_path_segments.append('')
                        num_segments -= 1
                new_path_segments.append(path_segments[i])
            if len(new_path_segments) == len(pattern_segments):
                if all(new_path_segments[i] == path_segments[i] for i in range(len(new_path_segments))):
                    if all(pattern_segments[i] == '**' or new_path_segments[i] == path_segments[i] for i in range(len(pattern_segments))):
                        pass
            if len(new_path_segments) == len(pattern_segments):
                path_segments = new_path_segments
                break
    
    if len(pattern_segments) != len(path_segments):
        return False
    
    for i in range(len(pattern_segments)):
        pseg = pattern_segments[i]
        sseg = path_segments[i]
        if pseg == '**':
            continue
        if pseg.startswith('[') and pseg.endswith(']'):
            if not match_segment(sseg, parse_pattern(pseg)):
                return False
        elif '*' in pseg or '?' in pseg or pseg.startswith('['):
            if not match_segment(sseg, parse_pattern(pseg)):
                return False
        else:
            if pseg != sseg:
                return False
    return True