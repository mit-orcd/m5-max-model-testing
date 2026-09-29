import re

def match(pattern, path):
    # Escape backslashes and handle character classes
    escaped_pattern = []
    i = 0
    n = len(pattern)
    while i < n:
        if pattern[i] == '\\':
            escaped_pattern.append(pattern[i+1])
            i += 2
        else:
            escaped_pattern.append(pattern[i])
            i += 1
    pattern = ''.join(escaped_pattern)

    # Split into segments
    pat_segments = pattern.split('/')
    path_segments = path.split('/')

    # Handle '**' segments
    star_indices = [i for i, seg in enumerate(pat_segments) if seg == '**']
    if not star_indices:
        # No '**' in pattern, simple segment matching
        if len(pat_segments) != len(path_segments):
            return False
        for pat_seg, path_seg in zip(pat_segments, path_segments):
            if not _match_segment(pat_seg, path_seg):
                return False
        return True
    else:
        # Handle '**' segments with state machine
        # States: 0 = before first '**', 1 = in '**', 2 = after last '**'
        state = 0
        pat_idx = 0
        path_idx = 0
        star_start = 0
        star_end = 0

        for i in range(len(pat_segments)):
            if pat_segments[i] == '**':
                if state == 0:
                    state = 1
                    star_start = i
                    # Match as many segments as possible before '**'
                    while pat_idx < star_start and path_idx < len(path_segments):
                        if not _match_segment(pat_segments[pat_idx], path_segments[path_idx]):
                            break
                        pat_idx += 1
                        path_idx += 1
                elif state == 1:
                    star_end = i
                    # Try to match remaining pattern after '**'
                    remaining_pat = pat_segments[star_end+1:]
                    remaining_path = path_segments[path_idx:]
                    if len(remaining_pat) > len(remaining_path):
                        return False
                    # Check if remaining pattern matches remaining path
                    for p, r in zip(remaining_pat, remaining_path):
                        if not _match_segment(p, r):
                            break
                    else:
                        return True
                    # If not, continue with more '**'
                    continue
                elif state == 2:
                    return False
            else:
                if state == 0:
                    if not _match_segment(pat_segments[i], path_segments[path_idx]):
                        return False
                    path_idx += 1
                elif state == 1:
                    # Try to match this segment with current pattern segment
                    if _match_segment(pat_segments[i], path_segments[path_idx]):
                        pat_idx = i + 1
                        path_idx += 1
                    # Also try consuming more segments (staying in '**')
                    star_end = i
                    remaining_pat = pat_segments[star_end+1:]
                    remaining_path = path_segments[path_idx:]
                    if len(remaining_pat) > len(remaining_path):
                        continue
                    for p, r in zip(remaining_pat, remaining_path):
                        if not _match_segment(p, r):
                            break
                    else:
                        return True
                elif state == 2:
                    if path_idx >= len(path_segments):
                        return False
                    if not _match_segment(pat_segments[i], path_segments[path_idx]):
                        return False
                    path_idx += 1
        # After processing all pattern segments
        if state == 1:
            # We're still in '**', so we can match remaining path segments
            return True
        elif state == 2:
            return path_idx == len(path_segments)
        return False

def _match_segment(pat_seg, path_seg):
    if pat_seg == '*':
        return True
    if pat_seg == '':
        return path_seg == ''
    if '?' in pat_seg or '[' in pat_seg or '*' in pat_seg:
        # Convert segment pattern to regex
        regex_parts = []
        i = 0
        n = len(pat_seg)
        while i < n:
            if pat_seg[i] == '?':
                regex_parts.append('.')
                i += 1
            elif pat_seg[i] == '*':
                regex_parts.append('.*')
                i += 1
            elif pat_seg[i] == '[':
                j = i + 1
                negate = False
                if pat_seg[j] == '!':
                    negate = True
                    j += 1
                chars = []
                while j < n and pat_seg[j] != ']':
                    if j + 1 < n and pat_seg[j+1] == '-':
                        chars.append((pat_seg[j], pat_seg[j+2]))
                        j += 3
                    else:
                        chars.append(pat_seg[j])
                        j += 1
                if j >= n:
                    return False  # Unclosed bracket
                # Build character class
                char_class = []
                for c in chars:
                    if isinstance(c, tuple):
                        start, end = c
                        char_class.extend(chr(ord(x)) for x in range(ord(start), ord(end)+1))
                    else:
                        char_class.append(chr(ord(c)))
                char_class = ''.join(char_class)
                if negate:
                    char_class = f'^{char_class}$
                    regex_parts.append(f'[^{char_class[:-1]}]')
                else:
                    regex_parts.append(f'[{char_class}]')
                i = j + 1
            else:
                regex_parts.append(re.escape(pat_seg[i]))
                i += 1
        regex = '^' + ''.join(regex_parts) + '$'
        return bool(re.fullmatch(regex, path_seg))
    return pat_seg == path_seg