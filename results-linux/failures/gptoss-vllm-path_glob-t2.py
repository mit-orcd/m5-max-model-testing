def match(pattern, path):
    # ------------------------------------------------------------------
    # parse a single pattern segment into a list of tokens
    # ------------------------------------------------------------------
    def parse_segment_pattern(pat):
        tokens = []
        i = 0
        n = len(pat)
        while i < n:
            c = pat[i]
            if c == '\\':
                i += 1
                if i < n:
                    tokens.append(('lit', pat[i]))
                    i += 1
                else:
                    tokens.append(('lit', '\\'))
            elif c == '?':
                tokens.append(('q',))
                i += 1
            elif c == '*':
                tokens.append(('star',))
                i += 1
            elif c == '[':
                i += 1
                neg = False
                if i < n and pat[i] == '!':
                    neg = True
                    i += 1
                char_set = []
                while i < n:
                    if pat[i] == '\\' and i + 1 < n:
                        i += 1
                        char_set.append(pat[i])
                        i += 1
                    elif pat[i] == ']':
                        i += 1
                        break
                    elif i + 2 < n and pat[i + 1] == '-' and pat[i + 2] != ']':
                        start = pat[i]
                        end = pat[i + 2]
                        for code in range(ord(start), ord(end) + 1):
                            char_set.append(chr(code))
                        i += 3
                    else:
                        char_set.append(pat[i])
                        i += 1
                tokens.append(('class', set(char_set), neg))
            else:
                tokens.append(('lit', c))
                i += 1
        return tokens

    # ------------------------------------------------------------------
    # parse the whole pattern into segments
    # ------------------------------------------------------------------
    def parse_pattern(pattern):
        segs = []
        curr = []
        escape = False
        for c in pattern:
            if escape:
                curr.append(c)
                escape = False
            elif c == '\\':
                escape = True
            elif c == '/':
                seg = ''.join(curr)
                if seg == '**':
                    segs.append('**')
                else:
                    segs.append(parse_segment_pattern(seg))
                curr = []
            else:
                curr.append(c)
        seg = ''.join(curr)
        if seg == '**':
            segs.append('**')
        else:
            segs.append(parse_segment_pattern(seg))
        return segs

    pattern_segs = parse_pattern(pattern)
    path_segs = path.split('/')

    # ------------------------------------------------------------------
    # match a single segment pattern against a path segment
    # ------------------------------------------------------------------
    def segment_match(tokens, seg):
        m = {}
        def rec(i, j):
            key = (i, j)
            if key in m:
                return m[key]
            if i == len(tokens):
                res = j == len(seg)
                m[key] = res
                return res
            if j == len(seg):
                # remaining tokens must all be stars
                for k in range(i, len(tokens)):
                    if tokens[k][0] != 'star':
                        m[key] = False
                        return False
                m[key] = True
                return True
            tok = tokens[i]
            if tok[0] == 'lit':
                res = seg[j] == tok[1] and rec(i + 1, j + 1)
            elif tok[0] == 'q':
                res = rec(i + 1, j + 1)
            elif tok[0] == 'star':
                # match zero or more chars
                res = rec(i + 1, j) or rec(i, j + 1)
            elif tok[0] == 'class':
                c = seg[j]
                ok = (c in tok[1]) != tok[2]
                res = ok and rec(i + 1, j + 1)
            else:
                res = False
            m[key] = res
            return res
        return rec(0, 0)

    # ------------------------------------------------------------------
    # overall match with '**' handling
    # ------------------------------------------------------------------
    memo = {}
    def rec(i, j):
        key = (i, j)
        if key in memo:
            return memo[key]
        if i == len(pattern_segs) and j == len(path_segs):
            memo[key] = True
            return True
        if i == len(pattern_segs):
            memo[key] = False
            return False
        pat = pattern_segs[i]
        if pat == '**':
            # try all possible numbers of segments to consume
            for k in range(j, len(path_segs) + 1):
                if rec(i + 1, k):
                    memo[key] = True
                    return True
            memo[key] = False
            return False
        else:
            if j >= len(path_segs):
                memo[key] = False
                return False
            if segment_match(pat, path_segs[j]) and rec(i + 1, j + 1):
                memo[key] = True
                return True
            memo[key] = False
            return False

    return rec(0, 0)