import re

def match(pattern, path):
    # Escape special regex characters except *, ?, [, \
    pattern_escaped = []
    i = 0
    n = len(pattern)
    while i < n:
        if pattern[i] == '\\':
            pattern_escaped.append(re.escape(pattern[i+1]))
            i += 2
        elif pattern[i] == '*':
            pattern_escaped.append('.*')
            i += 1
        elif pattern[i] == '?':
            pattern_escaped.append('.')
            i += 1
        elif pattern[i] == '[':
            # Handle character class
            j = i + 1
            negate = False
            if j < n and pattern[j] == '!':
                negate = True
                j += 1
            chars = []
            while j < n and pattern[j] != ']':
                if j + 2 < n and pattern[j+1] == '-' and pattern[j+2] != ']':
                    chars.append(f"{pattern[j]}-{pattern[j+2]}")
                    j += 3
                else:
                    chars.append(pattern[j])
                    j += 1
            if negate:
                chars_str = f"[{''.join(chars)}]"
                pattern_escaped.append(f"(?!{chars_str})[^\\/]")
            else:
                pattern_escaped.append(f"[{''.join(chars)}]")
            i = j + 1
        else:
            pattern_escaped.append(re.escape(pattern[i]))
            i += 1
    pattern_regex = '^' + ''.join(pattern_escaped) + '$'

    # Handle ** special case
    if '**' in pattern:
        segments_pattern = pattern.split('/')
        segments_path = path.split('/')
        for i in range(len(segments_pattern)):
            if segments_pattern[i] == '**':
                # Check remaining segments
                remaining_pattern = '/'.join(segments_pattern[i+1:])
                remaining_path = '/'.join(segments_path[i:])
                full_pattern = '/'.join(segments_pattern[:i]) + remaining_pattern
                full_path = '/'.join(segments_path[:i]) + remaining_path
                full_pattern_regex = '^' + re.escape('/'.join(segments_pattern[:i])) + pattern_regex[len('^' + re.escape('/'.join(segments_pattern[:i]))):].replace(r'\*', '.*') + '$'
                full_path_regex = '^' + re.escape(full_path) + '$'
                return bool(re.fullmatch(full_pattern_regex, full_path))
        return False

    # Standard regex matching
    return bool(re.fullmatch(pattern_regex, path))