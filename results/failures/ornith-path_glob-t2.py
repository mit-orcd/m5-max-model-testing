def match(pattern, path):
    def escape(char):
        if char is None:
            return False
        return char in r'\`$^*+?()[]{}|'

    def parse_class(chars):
        i = 0
        negate = False
        if i < len(chars) and chars[i] == '!':
            negate = True
            i += 1
        result = set()
        n = len(chars)
        while i < n:
            if chars[i] == '\\':
                if i + 1 < n:
                    result.add(chars[i + 1])
                    i += 2
                    continue
                i += 1
                continue
            if i + 2 < n and chars[i + 1] == '-' and chars[i + 2] != ']':
                start = chars[i]
                end = chars[i + 2]
                for code in range(ord(start), ord(end) + 1):
                    result.add(chr(code))
                i += 3
                continue
            result.add(chars[i])
            i += 1
        if negate:
            return lambda c: c not in result and c != '/'
        return lambda c: c in result and c != '/'

    def compile_segments(pattern):
        segments = []
        current = []
        i = 0
        n = len(pattern)
        while i < n:
            char = pattern[i]
            if char == '\\':
                if i + 1 < n:
                    current.append(('literal', pattern[i + 1]))
                    i += 2
                    continue
                i += 1
            elif char == '[':
                j = i + 1
                if j < n and pattern[j] == '!':
                    j += 1
                if j < n and pattern[j] == ']':
                    j += 1
                while j < n and pattern[j] != ']':
                    if pattern[j] == '\\':
                        j += 2
                    else:
                        j += 1
                if j >= n:
                    current.append(('literal', '['))
                    i += 1
                else:
                    cls = parse_class(pattern[i + 1:j])
                    current.append(('class', cls))
                    i = j + 1
            elif char == '*':
                if i + 1 < n and pattern[i + 1] == '*':
                    j = i
                    while j < n and pattern[j] == '*':
                        j += 1
                    if j < n and pattern[j] == '/':
                        if current:
                            segments.append(current)
                            current = []
                        if j + 1 < n and pattern[j + 1] == '/':
                            segments.append(('star-segment',))
                            j += 1
                        else:
                            segments.append(('star-segment-zero',))
                        i = j + 1
                        continue
                    else:
                        current.append(('star',))
                        i = j
                        continue
                current.append(('star',))
                i += 1
            elif char == '?':
                current.append(('question',))
                i += 1
            elif char == '/':
                segments.append(current)
                current = []
                i += 1
            else:
                current.append(('literal', char))
                i += 1
        if current:
            segments.append(current)
        return segments

    def match_segments(segments, path_segments):
        idx = 0
        i = 0
        while i < len(segments):
            seg = segments[i]
            if seg == ('star-segment',):
                rest = segments[i + 1:]
                for k in range(idx, len(path_segments) + 1):
                    if match_segments(rest, path_segments[idx:k]):
                        return True
                return False
            elif seg == ('star-segment-zero',):
                rest = segments[i + 1:]
                for k in range(idx, len(path_segments) + 1):
                    if match_segments(rest, path_segments[idx:k]):
                        return True
                return False
            if idx >= len(path_segments):
                return False
            if not match_one_segment(seg, path_segments[idx]):
                return False
            idx += 1
            i += 1
        return idx == len(path_segments)

    def match_one_segment(seg, text):
        i = 0
        n = len(text)
        for token in seg:
            kind = token[0]
            if kind == 'literal':
                if i >= n or text[i] != token[1]:
                    return False
                i += 1
            elif kind == 'class':
                if i >= n or not token[1](text[i]):
                    return False
                i += 1
            elif kind == 'question':
                if i >= n:
                    return False
                i += 1
            elif kind == 'star':
                for j in range(i, n + 1):
                    if match_rest(seg, j, text):
                        return True
                return False
        return i == n

    def match_rest(seg, start, text):
        idx = 1
        for token in seg[1:]:
            kind = token[0]
            if kind == 'literal':
                if start >= len(text) or text[start] != token[1]:
                    return False
                start += 1
            elif kind == 'class':
                if start >= len(text) or not token[1](text[start]):
                    return False
                start += 1
            elif kind == 'question':
                if start >= len(text):
                    return False
                start += 1
            elif kind == 'star':
                for j in range(start, len(text) + 1):
                    if match_rest(seg, j, text):
                        return True
                return False
        return start == len(text)

    if not isinstance(pattern, str) or not isinstance(path, str):
        return False
    segments = compile_segments(pattern)
    path_segments = path.split('/')
    return match_segments(segments, path_segments)