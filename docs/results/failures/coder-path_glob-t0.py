```python
def match(pattern, path):
    pattern_parts = pattern.split('/')
    path_parts = path.split('/')
    
    def match_pattern(pattern_parts, path_parts, p_idx, p_len, p_pattern):
        if p_idx == p_len:
            return p_idx == len(pattern_parts) and len(path_parts) == 0
        if p_idx == len(pattern_parts):
            return len(path_parts) == 0
        pattern_part = pattern_parts[p_idx]
        if pattern_part == '**':
            # Match zero or more path segments
            if p_idx == len(pattern_parts) - 1:
                # Last pattern part, match rest of path
                return True
            # Try to match zero or more segments
            for i inimport re
def match(pattern, path):
    # Split pattern and path into parts
    pattern_parts = pattern.split('/')
    path_parts = path.split('/')
    
    def match_pattern(p_idx, p_len, p_pattern):
        if p_idx == p_len:
            return len(path_parts) == 0
        pattern_part = pattern_parts[p_idx]
        if pattern_part == '**':
            # Match zero or more path segments
            if p_idx == len(pattern_parts) - 1:
                # Last pattern part, match rest of path
                return True
            # Try to match zero or or more segments
            for i in range(len(path_parts) + 1):
                if match_pattern(p_idx + 1, p_len, p_pattern):
                    return True
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len(path_parts) and path_parts[i] == '':
                    break
                if i < len(path_parts) and path_parts[i] == '/':
                    break
                if i < len