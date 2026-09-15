def match(pattern, path):
    def parse_pattern(pat):
        tokens = []
        i = 0
        n = len(pat)
        while i < n:
            c = pat[i]
            if c == '\\':
                if i + 1 < n:
                    tokens.append(('char', pat[i + 1]))
                    i += 2
                else:
                    tokens.append(('char', '\\'))
                    i += 1
            elif c == '*':
                tokens.append(('star',))
                i += 1
            elif c == '?':
                tokens.append(('question',))
                i += 1
            elif c == '[':
                j = i + 1
                if j < n and pat[j] == '!':
                    negate = True
                    j += 1
                else:
                    negate = False
                chars = []
                while j < n and pat[j] != ']':
                    if pat[j] == '\\' and j + 1 < n:
                        chars.append(pat[j + 1])
                        j += 2
                    else:
                        chars.append(pat[j])
                        j += 1
                if j >= n:
                    tokens.append(('char', '['))
                    i += 1
                else:
                    tokens.append(('class', negate, chars))
                    i = j + 1
            else:
                tokens.append(('char', c))
                i += 1
        return tokens

    def match_tokens(tokens, s):
        if not tokens:
            return s == ''
        if not s and not any(t[0] == 'star' for t in tokens):
            return False
        first = tokens[0]
        if first[0] == 'char':
            if s and s[0] == first[1]:
                return match_tokens(tokens[1:], s[1:])
            return False
        elif first[0] == 'question':
            if s and s[0] != '/':
                return match_tokens(tokens[1:], s[1:])
            return False
        elif first[0] == 'star':
            for i in range(0, len(s) + 1):
                if i < len(s) and s[i] == '/':
                    break
                if match_tokens(tokens[1:], s[i:]):
                    return True
            return False
        elif first[0] == 'class':
            negate = first[1]
            chars = first[2]
            if s and s[0] != '/':
                matched = s[0] in chars
                if (matched and not negate) or (not matched and negate):
                    return match_tokens(tokens[1:], s[1:])
            return False
        return False

    def match_segments(pattern_segs, path_segs):
        if not pattern_segs:
            return not path_segs
        p_seg = pattern_segs[0]
        if p_seg == ['**']:
            for i in range(len(path_segs) + 1):
                if match_segments(pattern_segs[1:], path_segs[i:]):
                    return True
            return False
        if not path_segs:
            return False
        if match_tokens(p_seg, path_segs[0]):
            return match_segments(pattern_segs[1:], path_segs[1:])
        return False

    pat_tokens = parse_pattern(pattern)
    pat_segs = []
    current = []
    for t in pat_tokens:
        if t == ('star',) or (t[0] == 'char' and t[1] == '/'):
            if current:
                pat_segs.append(current)
                current = []
            if t == ('star',):
                current.append(t)
            else:
                current = ['**']
                pat_segs.append(current)
                current = []
        else:
            current.append(t)
    if current:
        pat_segs.append(current)
    path_segs = path.split('/')
    return match_segments(pat_segs, path_segs)