import re

def match(pattern, path):
    # Escape backslashes and handle character classes
    escaped_pattern = []
    i = 0
    n = len(pattern)
    while i < n:
        if pattern[i] == '\\':
            if i + 1 < n:
                escaped_pattern.append(re.escape(pattern[i+1]))
                i += 2
            else:
                escaped_pattern.append('\\')
                i += 1
        elif pattern[i] == '[':
            j = i + 1
            negate = False
            if j < n and pattern[j] == '!':
                negate = True
                j += 1
            chars = []
            while j < n and pattern[j] != ']':
                if j + 1 < n and pattern[j+1] == '-' and j + 2 < n and pattern[j+2] != ']':
                    chars.append((pattern[j], pattern[j+2]))
                    j += 3
                else:
                    chars.append(pattern[j])
                    j += 1
            if negate:
                escaped_chars = ''.join(c if isinstance(c, str) else f'{c[0]}-{c[1]}' for c in chars)
                escaped_pattern.append(f'[^{escaped_chars}]')
            else:
                escaped_chars = ''.join(c if isinstance(c, str) else f'{c[0]}-{c[1]}' for c in chars)
                escaped_pattern.append(f'[{escaped_chars}]')
            i = j + 1
        else:
            escaped_pattern.append(pattern[i])
            i += 1
    escaped_pattern = ''.join(escaped_pattern)

    # Split into segments
    pattern_segments = pattern.split('/')
    path_segments = path.split('/')

    # Handle ** segments
    pattern_segments = [s for s in pattern_segments if s != '**']
    if '**' in pattern.split('/'):
        # Find the position of **
        star_pos = pattern.split('/').index('**')
        # Split into prefix and suffix
        prefix = pattern.split('/')[:star_pos]
        suffix = pattern.split('/')[star_pos+1:]
        # Check prefix
        if not all(match_segment(p, s) for p, s in zip(prefix, path_segments)):
            return False
        # Check suffix
        if len(suffix) > len(path_segments):
            return False
        if not all(match_segment(p, s) for p, s in zip(suffix, path_segments[-len(suffix):])):
            return False
        return True

    # No **, so match exactly
    if len(pattern_segments) != len(path_segments):
        return False
    return all(match_segment(p, s) for p, s in zip(pattern_segments, path_segments))

def match_segment(pattern, segment):
    # Escape special regex characters except * and ?
    regex = []
    i = 0
    n = len(pattern)
    while i < n:
        if pattern[i] == '*':
            regex.append('.*')
            i += 1
        elif pattern[i] == '?':
            regex.append('.')
            i += 1
        elif pattern[i] == '[':
            j = i + 1
            negate = False
            if j < n and pattern[j] == '!':
                negate = True
                j += 1
            chars = []
            while j < n and pattern[j] != ']':
                if j + 1 < n and pattern[j+1] == '-' and j + 2 < n and pattern[j+2] != ']':
                    chars.append((pattern[j], pattern[j+2]))
                    j += 3
                else:
                    chars.append(pattern[j])
                    j += 1
            if negate:
                escaped_chars = ''.join(c if isinstance(c, str) else f'{c[0]}-{c[1]}' for c in chars)
                regex.append(f'[^{escaped_chars}]')
            else:
                escaped_chars = ''.join(c if isinstance(c, str) else f'{c[0]}-{c[1]}' for c in chars)
                regex.append(f'[{escaped_chars}]')
            i = j + 1
        else:
            regex.append(re.escape(pattern[i]))
            i += 1
    regex = ''.join(regex)
    return bool(re.fullmatch(regex, segment))