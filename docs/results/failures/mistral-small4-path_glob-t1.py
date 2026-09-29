import re

def match(pattern, path):
    pattern_parts = pattern.split('/')
    path_parts = path.split('/')

    def escape_glob(glob):
        escaped = []
        i = 0
        while i < len(glob):
            if glob[i] == '\\':
                if i + 1 < len(glob):
                    escaped.append(re.escape(glob[i+1]))
                    i += 2
                else:
                    escaped.append('\\')
                    i += 1
            elif glob[i] == '?':
                escaped.append('[^/]')
                i += 1
            elif glob[i] == '*':
                if i + 1 < len(glob) and glob[i+1] == '*':
                    escaped.append('.*')
                    i += 2
                else:
                    escaped.append('[^/]*')
                    i += 1
            else:
                escaped.append(glob[i])
                i += 1
        return ''.join(escaped)

    def compile_class(s):
        if s.startswith('[!'):
            return '[^' + escape_class(s[2:-1]) + ']'
        elif s.startswith('['):
            return '[' + escape_class(s[1:-1]) + ']'
        else:
            return re.escape(s)

    def escape_class(cls):
        result = []
        i = 0
        while i < len(cls):
            if cls[i] == '\\':
                if i + 1 < len(cls):
                    result.append(re.escape(cls[i+1]))
                    i += 2
                else:
                    result.append('\\')
                    i += 1
            elif cls[i] == ']':
                result.append('\\]')
                i += 1
            else:
                result.append(cls[i])
                i += 1
        return ''.join(result)

    def glob_to_regex(glob):
        if not glob:
            return ''
        parts = []
        for part in glob.split('/'):
            if part == '**':
                parts.append('.*')
            elif '**' in part:
                raise ValueError('Invalid pattern')
            elif part == '*':
                parts.append('[^/]*')
            elif '*' in part:
                parts.append(re.escape(part).replace('\\*', '[^/]*'))
            elif part == '?':
                parts.append('[^/]')
            elif '?' in part:
                parts.append(glob_to_regex(part))
            else:
                parts.append(compile_class(part))
        pattern = '/'.join(parts)
        pattern = '^' + pattern + '$'
        return re.compile(pattern)

    def glob_to_regex(part):
        if part == '**':
            return '.*'
        elif part.startswith('[!'):
            return '[^' + escape_class(part[2:-1]) + ']'
        elif part.startswith('['):
            return '[' + escape_class(part[1:-1]) + ']'
        else:
            return re.escape(part)

    def escape_class(cls):
        result = []
        i = 0
        while i < len(cls):
            if cls[i] == '\\':
                if i + 1 < len(cls):
                    result.append(re.escape(cls[i+1]))
                    i += 2
                else:
                    result.append('\\')
                    i += 1
            elif cls[i] == ']':
                result.append('\\]')
                i += 1
            else:
                result.append(cls[i])
                i += 1
        return ''.join(result)

    if len(path_parts) < len(pattern_parts):
        return False

    if '*' in pattern and '**' not in pattern:
        regex = glob_to_regex(pattern)
        return bool(regex.match(path))
    else:
        i = 0
        while i < len(path_parts):
            if i >= len(pattern_parts):
                return False
            if path_parts[i] != pattern_parts[i]:
                if pattern_parts[i] == '**':
                    return True
                elif '**' in pattern_parts[i]:
                    return False
                regex = glob_to_regex(pattern_parts[i])
                if not regex.match(path_parts[i]):
                    return False
                i += 1
            else:
                if pattern_parts[i] == '**':
                    if i == len(path_parts) - 1:
                        return True
                    if i + 1 < len(pattern_parts) and pattern_parts[i+1] == '**':
                        i += 1
                    else:
                        i += 1
                        if i >= len(path_parts):
                            return True
                else:
                    regex = glob_to_regex(pattern_parts[i])
                    if not regex.match(path_parts[i]):
                        return False
                    i += 1
        return i == len(pattern_parts)

def glob_to_regex(glob):
    if not glob:
        return re.compile('^$')
    parts = []
    i = 0
    while i < len(glob):
        if glob[i] == '\\':
            if i + 1 < len(glob):
                parts.append(re.escape(glob[i+1]))
                i += 2
            else:
                parts.append('\\')
                i += 1
        elif glob[i] == '?':
            parts.append('[^/]')
            i += 1
        elif glob[i] == '*':
            if i + 1 < len(glob) and glob[i+1] == '*':
                parts.append('.*')
                i += 2
            else:
                parts.append('[^/]*')
                i += 1
        else:
            parts.append(glob[i])
            i += 1
    regex = ''.join(parts)
    return re.compile('^' + regex + '$')

def compile_class(s):
    if s.startswith('[!'):
        return '[^' + escape_class(s[2:-1]) + ']'
    elif s.startswith('['):
        return '[' + escape_class(s[1:-1]) + ']'
    else:
        return re.escape(s)

def escape_class(cls):
    result = []
    i = 0
    while i < len(cls):
        if cls[i] == '\\':
            if i + 1 < len(cls):
                result.append(re.escape(cls[i+1]))
                i += 2
            else:
                result.append('\\')
                i += 1
        elif cls[i] == ']':
            result.append('\\]')
            i += 1
        else:
            result.append(cls[i])
            i += 1
    return ''.join(result)

def match(pattern, path):
    pattern_parts = pattern.split('/')
    path_parts = path.split('/')

    def escape_glob(glob):
        escaped = []
        i = 0
        while i < len(glob):
            if glob[i] == '\\':
                if i + 1 < len(glob):
                    escaped.append(re.escape(glob[i+1]))
                    i += 2
                else:
                    escaped.append('\\')
                    i += 1
            elif glob[i] == '?':
                escaped.append('[^/]')
                i += 1
            elif glob[i] == '*':
                if i + 1 < len(glob) and glob[i+1] == '*':
                    escaped.append('.*')
                    i += 2
                else:
                    escaped.append('[^/]*')
                    i += 1
            else:
                escaped.append(glob[i])
                i += 1
        return ''.join(escaped)

    def compile_pattern(parts):
        if not parts:
            return re.compile('^$')
        pattern = []
        for part in parts:
            if part == '**':
                pattern.append('.*')
            elif part.startswith('[!'):
                pattern.append('[^' + escape_class(part[2:-1]) + ']')
            elif part.startswith('['):
                pattern.append('[' + escape_class(part[1:-1]) + ']')
            elif '*' in part or '?' in part:
                pattern.append(escape_glob(part))
            else:
                pattern.append(re.escape(part))
        return re.compile('/'.join(pattern) + '$')

    def escape_class(cls):
        result = []
        i = 0
        while i < len(cls):
            if cls[i] == '\\':
                if i + 1 < len(cls):
                    result.append(re.escape(cls[i+1]))
                    i += 2
                else:
                    result.append('\\')
                    i += 1
            elif cls[i] == ']':
                result.append('\\]')
                i += 1
            else:
                result.append(cls[i])
                i += 1
        return ''.join(result)

    regex_pattern = compile_pattern(pattern_parts)
    if len(path_parts) != len(pattern_parts):
        if '**' not in pattern:
            return False
        for i in range(len(path_parts)):
            if i >= len(pattern_parts):
                return False
            test_path = path_parts[:i] + [''] + path_parts[i:]
            joined = '/'.join(test_path)
            if regex_pattern.match(joined):
                return True
        return False
    else:
        joined_path = '/'.join(path_parts)
        return bool(regex_pattern.match(joined_path))