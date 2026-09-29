import re

def match(pattern, path):
    def escape_regex(s):
        return re.escape(s)

    def parse_pattern(p):
        i = 0
        n = len(p)
        parts = []
        while i < n:
            if p[i] == '\\':
                i += 1
                if i < n:
                    parts.append(('lit', p[i]))
                    i += 1
                else:
                    parts.append(('lit', '\\'))
            elif p[i] == '*':
                if i + 1 < n and p[i+1] == '*':
                    parts.append(('doublestar',))
                    i += 2
                else:
                    parts.append(('star',))
                    i += 1
            elif p[i] == '?':
                parts.append(('qmark',))
                i += 1
            elif p[i] == '[':
                j = i + 1
                if j < n and p[j] == '!':
                    j += 1
                    negate = True
                else:
                    negate = False
                chars = []
                while j < n and p[j] != ']':
                    if j + 2 < n and p[j+1] == '-':
                        chars.append((p[j], p[j+2]))
                        j += 3
                    else:
                        chars.append(p[j])
                        j += 1
                if j >= n or p[j] != ']':
                    parts.append(('lit', '['))
                    i += 1
                    continue
                parts.append(('class', chars, negate))
                i = j + 1
            else:
                parts.append(('lit', p[i]))
                i += 1
        return parts

    def build_regex(parts):
        regex_parts = ['^']
        for part in parts:
            if part[0] == 'lit':
                regex_parts.append(escape_regex(part[1]))
            elif part[0] == 'qmark':
                regex_parts.append('[^/]')
            elif part[0] == 'star':
                regex_parts.append('[^/]*')
            elif part[0] == 'doublestar':
                regex_parts.append('(?:[^/]*/*)*')
            elif part[0] == 'class':
                chars, negate = part[1], part[2]
                if not chars:
                    class_str = ''
                else:
                    class_parts = []
                    for c in chars:
                        if isinstance(c, tuple):
                            start, end = c
                            class_parts.append(f'{escape_regex(start)}-{escape_regex(end)}')
                        else:
                            class_parts.append(escape_regex(c))
                    class_str = ''.join(class_parts)
                if negate:
                    regex_parts.append(f'[^{class_str}]')
                else:
                    regex_parts.append(f'[{class_str}]')
        regex_parts.append('$')
        return ''.join(regex_parts)

    pattern_parts = parse_pattern(pattern)
    regex_str = build_regex(pattern_parts)
    try:
        regex = re.compile(regex_str)
    except re.error:
        return False
    return bool(regex.match(path))