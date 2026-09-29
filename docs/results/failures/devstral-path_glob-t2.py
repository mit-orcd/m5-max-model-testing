def match(pattern, path):
    import re
    pattern_parts = pattern.split('/')
    path_parts = path.split('/')
    
    def escape_backslashes(s):
        return s.replace('\\', '\\\\')
    
    def convert_to_regex(segment):
        regex = []
        i = 0
        n = len(segment)
        while i < n:
            if segment[i] == '\\':
                regex.append(escape_backslashes(segment[i+1]))
                i += 2
            elif segment[i] == '?':
                regex.append('.')
                i += 1
            elif segment[i] == '*':
                regex.append('.*')
                i += 1
            elif segment[i] == '[':
                j = i + 1
                while j < n and segment[j] != ']':
                    j += 1
                if j >= n:
                    raise ValueError("Invalid pattern: unclosed '['")
                char_class = segment[i+1:j]
                if char_class.startswith('!'):
                    char_class = char_class[1:]
                    regex.append('(?![^' + char_class.replace('-', '\\-') + '])')
                else:
                    regex.append('[' + char_class.replace('-', '\\-') + ']')
                i = j + 1
            else:
                regex.append(escape_backslashes(segment[i]))
                i += 1
        return ''.join(regex)
    
    if '**' in pattern_parts:
        double_star_index = pattern_parts.index('**')
        left_pattern = pattern_parts[:double_star_index]
        right_pattern = pattern_parts[double_star_index+1:]
        
        left_regex = '^' + '.*'.join([convert_to_regex(p) for p in left_pattern]) + '$'
        right_regex = '^' + '.*'.join([convert_to_regex(p) for p in right_pattern]) + '$'
        
        left_match = re.fullmatch(left_regex, path_parts[0])
        if not left_match:
            return False
        
        remaining_path = path_parts[1:]
        right_match = re.fullmatch(right_regex, remaining_path)
        return bool(right_match)
    else:
        if len(pattern_parts) != len(path_parts):
            return False
        for p, s in zip(pattern_parts, path_parts):
            regex = '^' + convert_to_regex(p) + '$'
            if not re.fullmatch(regex, s):
                return False
        return True