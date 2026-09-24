import re

def match(pattern, path):
    def escape_regex(s):
        return re.escape(s)

    def parse_bracket(content):
        if not content:
            return False
        negate = content.startswith('!')
        if negate:
            content = content[1:]
        if not content:
            return False
        i = 0
        parts = []
        while i < len(content):
            if i + 2 < len(content) and content[i+1] == '-':
                start, end = content[i], content[i+2]
                if start > end:
                    return False
                parts.append(f"{start}-{end}")
                i += 3
            else:
                parts.append(re.escape(content[i]))
                i += 1
        if not parts:
            return False
        inner = ''.join(parts)
        if negate:
            return f"[^{inner}]"
        else:
            return f"[{inner}]"

    tokens = []
    i = 0
    n = len(pattern)
    while i < n:
        c = pattern[i]
        if c == '\\':
            i += 1
            if i < n:
                tokens.append(('LIT', pattern[i]))
                i += 1
            else:
                tokens.append(('LIT', '\\'))
        elif c == '*':
            if i + 1 < n and pattern[i+1] == '*':
                tokens.append(('DBLSTAR', None))
                i += 2
            else:
                tokens.append(('STAR', None))
                i += 1
        elif c == '?':
            tokens.append(('QUEST', None))
            i += 1
        elif c == '[':
            j = i + 1
            if j < n and pattern[j] == '!':
                j += 1
            while j < n and pattern[j] != ']':
                j += 1
            if j >= n:
                tokens.append(('LIT', '['))
                i += 1
            else:
                bracket_content = pattern[i+1:j]
                parsed = parse_bracket(bracket_content)
                if parsed is False:
                    tokens.append(('LIT', '['))
                    i += 1
                else:
                    tokens.append(('BRACKET', parsed))
                    i = j + 1
        else:
            tokens.append(('LIT', c))
            i += 1

    regex_parts = ['^']
    for typ, val in tokens:
        if typ == 'LIT':
            regex_parts.append(re.escape(val))
        elif typ == 'QUEST':
            regex_parts.append('[^/]')
        elif typ == 'STAR':
            regex_parts.append('[^/]*')
        elif typ == 'DBLSTAR':
            regex_parts.append('(?:[^/]+/)*')
        elif typ == 'BRACKET':
            regex_parts.append(val)
    regex_parts.append('$')

    regex = ''.join(regex_parts)
    try:
        return bool(re.fullmatch(regex, path))
    except re.error:
        return False