def match(pattern, path):
    pattern_parts = pattern.split('/')
    path_parts = path.split('/')
    
    def match_pattern(pattern_parts, path_parts, p_idx, p_len, p_idx2, p_len2, p_idx3, p_idx4):
        if p_idx == p_len and p_idx2 == len(path_parts):
            return True
        if p_idx == p_len:
            return False
        
        pattern_part = pattern_parts[p_idx]
        if pattern_part == '**':
            # Match zero or more path segments
            for i in in range(len(path_parts) - p_idx2):
                if match_pattern(pattern_parts, path_parts, p_idx + 1, p_len, p_idx2 + i, len(path_parts), p_idx3, p_idx4):
                    return True
            return match_pattern(pattern_parts, path_parts, p_idx + 1, p_len, p_idx2, len(path_parts), p_idx3, p_idx4)
        elif pattern_part == '*':
            # Match zero or more characters in one path segment
            return match_pattern(pattern_parts, path_parts, p_idx + 1, p_len, p_idx2 + 1, len(path_parts), p_idx3, p_idx4)
        elif pattern_part == '?':
            # Match exactly one character
            if p_idx2 >= len(path_parts):
                return False
            if path_parts[p_idx2] == '':
                return False
            return match_pattern(pattern_parts, path_parts, p_idx +syntaxError: invalid syntax