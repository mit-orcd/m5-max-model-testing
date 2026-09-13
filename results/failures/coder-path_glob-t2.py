def match(pattern, path):
    def is_match(p_pattern, p_path):
        if not p_pattern and not p_path:
            return True
        if not p_pattern or not p_path:
            return False
        if p_pattern[0] == '*':
            # Try to match zero or more characters
            return is_match(p_pattern[1:], p_path) or is_match(p_pattern, p_path[1:])
        elif p_pattern[0] == '?':
            # Match exactly one character
            return p_path and p_path[const] != '/' and is_match(p_pattern[1:], p_path[1:])
        elif p_pattern[0] == '[':
            # Character class
            if not p_path:
                return False
            class_start = 1
            negate = False
            if p_pattern[1] == '!':
                negate = True
                class_start = 2
            char_class = []
            i = class_start
            while i < len(p_pattern) and p_pattern[i] != ']':
                if i + 2 < len(p_pattern) and p_pattern[i+1] == '-' and p_pattern[i+2] != ']':
                    # Range like a-z
                    char_class.extend(chr(c) for c in range(ord(p_pattern[i]), ord(p_pattern[i+2])+1))
                    i += 3
                else:
                    char_class.append(p_pattern[i])
                    i += 1
            match_char = p_path[0] in char_class
            return match_char != negate and is_match(p_pattern[i+1:], p_path[1:])
        elif p_pattern[0] == '**':
            # Match zero or more segments
            if len(p_pattern) == 1:
                return True
            # Try all possible matches for ** pattern
            for i in range(len(p_path)+1):
                if is_match(p_pattern[2:], p_path[i:]) or (i < len(p_path) and p_path[i] == '/' and is_match(p_pattern[1:], p_path[i+1:])):
                    return True
            return False
        else:
            # Literal match
            if p_pattern[0] == p_path[0]:
                return is_match(p_pattern[1:], p_path[1:])
            return False

    path_segments = path.split('/')
    pattern_segments = pattern.split('/')

    # Handle ** pattern at end where it matches zero or more segments
    if len(pattern_segments) >= 2 and pattern_segments[-2] == '**':
        # Special handling for trailing "**"
        pattern = pattern[:len(pattern)-len('**')-1]
        pattern_segments = pattern.split('/')
        return is_match(pattern_segments + ['**'], path_segments)
    elif len(pattern_segments) == 1 and pattern == '**':
        return True
    else:
        return is_match(pattern_segments, path_segments)

def is_match(pattern, path):
    if not pattern and not path:
        return True
    if not pattern or not path:
        return False
    if pattern[0] == '*':
        return is_match(pattern[1:], path) or is_match(pattern, path[1:])
    elif pattern[0] == '?':
        return path and path[0] != '/' and is_match(pattern[1:], path[1:])
    elif pattern[0] == '[':
        if not path:
            return False
        i = 1
        negate = False
        if i < len(pattern) and pattern[i] == '!':
            negate = True
            i += 1
        char_class = []
        while i < len(pattern) and pattern[i] != ']':
            if i+2 < len(pattern) and pattern[i+1] == '-':
                # Range like a-z
                start = ord(pattern[i])
                end = ord(pattern[i+2])
                char_class.extend(chr(c) for c in range(start, end+1))
                i += 3
            else:
                char_class.append(pattern[i])
                i += 1
        match_char = path[0] in char_class
        return match_char != negate and is_match(pattern[i+1:], path[1:])
    else:
        if pattern[0] == path[0]:
            return is_match(pattern[1:], path[1:])
        return False

def match(pattern, path):
    path_segments = path.split('/')
    pattern_segments = pattern.split('/')
    
    def is_match(p_pattern, p_path):
        if not p_pattern and not p_path:
            return True
        if not p_pattern or not p_path:
            return False
        if p_pattern[0] == '**':
            # Match zero or or more segments
            if len(p_pattern) == 1:
                return True
            # Try all possible matches for ** pattern
            for i in range(len(p_path)+1):
                if is_match(p_pattern[1:], p_path[i:]) or (i < len(p_path) and p_path[i] == '/' and is_match(p_pattern[1:], p_path[i+1])):
                    return True
            return False
        elif p_pattern[0] == '*':
            # Match zero or more characters (but not '/')
            return is_match(p_pattern[1:], p_path) or (p_path and p_path[0] != '/' and is_match(p_pattern, p_path[1:])
        elif p_pattern[0] == '?':
            # Match exactly one character (but not '/')
            return p_path and p_path[0] != '/' and is_match(p_pattern[1:], p_path[1:])
        elif p_pattern[0] == '[':
            # Character class
            if not p_path:
                return False
            i = 1
            negate = False
            if i < len(p_pattern) and p_pattern[i] == '!':
                negate = True
                i += 1
            char_class = []
            while i < len(p_pattern) and p_pattern[i] != ']':
                if i+2 < len(p_pattern) and p_pattern[i+1] == '-':
                    # Range like a-z
                    start = ord(p_pattern[i])
                    end = ord(p_pattern[i+2])
                    char_class.extend(chr(c) for c in range(start, end+1))
                    i += 3
                else:
                    char_class.append(p_pattern[i])
                    i += 1
            match_char = p_path[0] in char_class
            return match_char != negate and is_match(p_pattern[i+1:], p_path[1:])
        else:
            # Literal match
            if p_pattern[0] == p_path[0]:
                return is_match(p_pattern[1:], p_path[1:])
            return False

    return is_match(pattern_segments, path_segments)