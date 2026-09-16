import re

def match(pattern, path):
    if not pattern and not path:
        return True
    if not pattern:
        return False

    parts = pattern.split('/')
    path_parts = path.split('/')

    # Preprocess pattern parts to handle ** and escape
    i = 0
    processed = []
    while i < len(parts):
        part = parts[i]
        if part == '**':
            # Merge consecutive ** into one
            while i + 1 < len(parts) and parts[i + 1] == '**':
                i += 1
            processed.append('**')
        else:
            # Escape backslashes and translate to regex
            escaped = re.escape(part)
            # Unescape ? and * and [ ] that are not escaped
            # We'll handle special chars manually after escaping
            # Instead: build regex manually for the part
            regex_part = ''
            j = 0
            while j < len(part):
                if part[j] == '\\':
                    if j + 1 < len(part):
                        regex_part += re.escape(part[j+1])
                        j += 2
                    else:
                        regex_part += '\\\\'
                        j += 1
                elif part[j] == '?':
                    regex_part += '[^/]'
                    j += 1
                elif part[j] == '*':
                    regex_part += '[^/]*'
                    j += 1
                elif part[j] == '[':
                    k = j + 1
                    negate = False
                    if k < len(part) and part[k] == '!':
                        negate = True
                        k += 1
                    if k < len(part) and part[k] == ']':
                        k += 1  # allow [] as literal
                    while k < len(part) and part[k] != ']':
                        k += 1
                    if k >= len(part):
                        regex_part += re.escape('[')  # unclosed, treat literally
                        j += 1
                    else:
                        content = part[j+1:k]
                        if negate:
                            content = '^' + content
                        regex_part += '[' + content + ']'
                        j = k + 1
                else:
                    regex_part += re.escape(part[j])
                    j += 1
            processed.append(regex_part)
        i += 1

    # Build full regex
    regex_parts = []
    for p in processed:
        if p == '**':
            regex_parts.append('(?:[^/]*/*)*')
        else:
            regex_parts.append(p)
    regex_str = '^' + '/'.join(regex_parts) + '$'
    try:
        return bool(re.fullmatch(regex_str, path))
    except re.error:
        return False