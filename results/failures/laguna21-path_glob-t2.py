def match(pattern, path):
    def parse_pattern(p):
        tokens = []
        i = 0
        while i < len(p):
            if p[i] == '\\' and i + 1 < len(p):
                tokens.append(('literal', p[i + 1]))
                i += 2
            elif p[i] == '[':
                j = i + 1
                negated = False
                if j < len(p) and p[j] == '!':
                    negated = True
                    j += 1
                chars = set()
                while j < len(p) and p[j] != ']':
                    if j + 2 < len(p) and p[j + 1] == '-':
                        chars.add(p[j])
                        chars.add(p[j + 2])
                        j += 3
                    else:
                        chars.add(p[j])
                        j += 1
                if j < len(p):
                    j += 1
                tokens.append(('class', chars, negated))
                i = j
            elif p[i] == '?':
                tokens.append(('question',))
                i += 1
            elif p[i] == '*':
                tokens.append(('star',))
                i += 1
            elif p[i] == '/':
                tokens.append(('slash',))
                i += 1
            else:
                tokens.append(('literal', p[i]))
                i += 1
        return tokens

    def parse_path(s):
        segments = s.split('/')
        return segments

    def match_segment(segment, pattern_tokens, pos):
        if not pattern_tokens and pos == len(segment):
            return True
        if pos >= len(segment):
            return False
        if not pattern_tokens:
            return False

        token = pattern_tokens[0]
        rest = pattern_tokens[1:]

        if token[0] == 'literal':
            if segment[pos] == token[1]:
                return match_segment(segment, rest, pos + 1)
            return False

        if token[0] == 'question':
            if segment[pos] != '/':
                return match_segment(segment, rest, pos + 1)
            return False

        if token[0] == 'class':
            ch = segment[pos]
            matched = ch in token[1]
            if token[2]:
                matched = not matched
            if matched:
                return match_segment(segment, rest, pos + 1)
            return False

        if token[0] == 'slash':
            if segment[pos] == '/':
                return match_segment(segment, rest, pos + 1)
            return False

        return False

    def match_path_segments(segments, pattern_tokens):
        if not pattern_tokens:
            return len(segments) == 0

        token = pattern_tokens[0]
        rest = pattern_tokens[1:]

        if token[0] == 'star':
            for i in range(len(segments) + 1):
                if match_path_segments(segments[i:], rest):
                    return True
            return False

        if token[0] == 'literal':
            if segments and segments[0] == token[1]:
                return match_path_segments(segments[1:], rest)
            return False

        if token[0] == 'question':
            if segments and segments[0] != '/':
                return match_path_segments(segments[1:], rest)
            return False

        if token[0] == 'class':
            if segments:
                ch = segments[0]
                matched = ch in token[1]
                if token[2]:
                    matched = not matched
                if matched:
                    return match_path_segments(segments[1:], rest)
            return False

        if token[0] == 'slash':
            return False

        return False

    if pattern == path:
        return True

    pattern_tokens = parse_pattern(pattern)
    path_segments = parse_path(path)

    pattern_segments = []
    current_segment = []
    for tok in pattern_tokens:
        if tok[0] == 'slash':
            pattern_segments.append(current_segment)
            current_segment = []
        else:
            current_segment.append(tok)
    pattern_segments.append(current_segment)

    def match_segments_list(segs, pat_segs):
        if not pat_segs and not segs:
            return True
        if not pat_segs or not segs:
            return False

        pat_tok = pat_segs[0]
        rest_pat = pat_segs[1:]

        if pat_tok[0] == 'star':
            for i in range(len(segs) + 1):
                if match_segments_list(segs[i:], rest_pat):
                    return True
            return False

        seg = segs[0]
        rest_seg = segs[1:]

        if pat_tok[0] == 'literal':
            if seg == pat_tok[1]:
                return match_segments_list(rest_seg, rest_pat)
            return False

        if pat_tok[0] == 'question':
            if len(seg) == 1 and seg != '/':
                return match_segments_list(rest_seg, rest_pat)
            return False

        if pat_tok[0] == 'class':
            if len(seg) == 1:
                ch = seg
                matched = ch in pat_tok[1]
                if pat_tok[2]:
                    matched = not matched
                if matched:
                    return match_segments_list(rest_seg, rest_pat)
            return False

        if pat_tok[0] == 'slash':
            return False

        return False

    return match_segments_list(path_segments, pattern_segments)