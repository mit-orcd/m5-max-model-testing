import re
from typing import List

def match(pattern: str, path: str) -> bool:
    def split_preserve_empty(s: str, sep: str) -> List[str]:
        parts = s.split(sep)
        if s.endswith(sep):
            parts.append('')
        return parts

    def compile_segment_pattern(seg: str) -> re.Pattern:
        i = 0
        n = len(seg)
        regex_parts = ['^']
        while i < n:
            c = seg[i]
            if c == '\\':
                i += 1
                if i < n:
                    regex_parts.append(re.escape(seg[i]))
                else:
                    regex_parts.append(re.escape('\\'))
                i += 1
            elif c == '?':
                regex_parts.append('[^/]')
                i += 1
            elif c == '*':
                regex_parts.append('[^/]*')
                i += 1
            elif c == '[':
                j = i + 1
                if j < n and seg[j] == '!':
                    j += 1
                    negate = True
                else:
                    negate = False
                if j >= n:
                    regex_parts.append(re.escape('['))
                    i += 1
                    continue
                charset = []
                while j < n and seg[j] != ']':
                    if j + 2 < n and seg[j+1] == '-':
                        charset.append(seg[j])
                        charset.append('-')
                        charset.append(seg[j+2])
                        j += 3
                    else:
                        charset.append(seg[j])
                        j += 1
                if j >= n:
                    regex_parts.append(re.escape('['))
                    i = n
                else:
                    inner = ''.join(charset)
                    if negate:
                        inner = '^' + inner
                    regex_parts.append('[' + inner + ']')
                    i = j + 1
            else:
                regex_parts.append(re.escape(c))
                i += 1
        regex_parts.append('$')
        return re.compile(''.join(regex_parts))

    pattern_parts = split_preserve_empty(pattern, '/')
    path_parts = split_preserve_empty(path, '/')

    i = j = 0
    star_star_pos = -1
    path_j_backup = -1

    while i < len(pattern_parts):
        if j < len(path_parts) and pattern_parts[i] == '**':
            star_star_pos = i
            path_j_backup = j
            i += 1
            continue
        if star_star_pos != -1:
            i = star_star_pos + 1
            j = path_j_backup + 1
            path_j_backup += 1
            if path_j_backup > len(path_parts):
                return False
            continue
        if i >= len(pattern_parts) or j >= len(path_parts):
            break
        pat_seg = pattern_parts[i]
        path_seg = path_parts[j]
        if pat_seg == '**':
            i += 1
            continue
        regex = compile_segment_pattern(pat_seg)
        if not regex.fullmatch(path_seg):
            return False
        i += 1
        j += 1

    while i < len(pattern_parts) and pattern_parts[i] == '**':
        i += 1

    return i == len(pattern_parts) and j == len(path_parts)