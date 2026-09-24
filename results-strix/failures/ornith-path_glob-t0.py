def match(pattern, path):
    def parse_segment(seg):
        tokens = []
        i = 0
        n = len(seg)
        while i < n:
            c = seg[i]
            if c == '\\':
                if i + 1 < n:
                    tokens.append(('char', seg[i + 1]))
                    i += 2
                else:
                    tokens.append(('char', '\\'))
                    i += 1
            elif c == '[':
                j = i + 1
                neg = False
                if j < n and seg[j] == '!':
                    neg = True
                    j += 1
                chars = set()
                ranges = []
                closed = False
                while j < n and seg[j] != ']':
                    ch = seg[j]
                    if ch == '\\' and j + 1 < n:
                        ch = seg[j + 1]
                        j += 2
                    else:
                        j += 1
                    if j + 1 < n and seg[j] == '-' and seg[j + 1] != ']':
                        start = ord(ch)
                        j += 1
                        endch = seg[j + 1]
                        if endch == '\\' and j + 2 < n:
                            endch = seg[j + 2]
                            j += 3
                        else:
                            j += 2
                        end = ord(endch)
                        if start <= end:
                            ranges.append((start, end))
                        else:
                            ranges.append((end, start))
                    else:
                        chars.add(ord(ch))
                j += 1
                closed = True
                tokens.append(('class', chars, ranges, neg, c))
            elif c == '*':
                j = i
                while j < n and seg[j] == '*':
                    j += 1
                if j - i >= 2 and (j == n or seg[j] == '/'):
                    count = j - i
                    tokens.append(('starstar', count))
                    i = j
                else:
                    tokens.append(('star',))
                    i = j
            else:
                tokens.append(('char', c))
                i += 1
        return tokens

    segments = path.split('/')
    pat_segments = pattern.split('/')
    if len(segments) != len(pat_segments):
        return False

    def match_segments(seg_tokens, seg_str):
        # seg_tokens: list of token tuples for one pattern segment
        # seg_str: the path segment string
        # returns list of possible (consumed_count) via DP
        # We'll do recursive matching with memoization
        from functools import lru_cache

        def rec(i, k):
            if (i, k) in memo:
                return memo[(i, k)]
            result = False
            if i == len(seg_tokens):
                result = (k == len(seg_str))
            else:
                tok = seg_tokens[i]
                if tok[0] == 'char':
                    if k < len(seg_str) and seg_str[k] == tok[1]:
                        result = rec(i + 1, k + 1)
                elif tok[0] == 'class':
                    _, chars, ranges, neg, ob = tok
                    if k < len(seg_str):
                        ch = seg_str[k]
                        if ch == '/':
                            result = False
                        else:
                            in_class = ch in chars or any(lo <= ord(ch) <= hi for lo, hi in ranges)
                            if neg:
                                in_class = not in_class
                            if in_class:
                                result = rec(i + 1, k + 1)
                elif tok[0] == 'star':
                    # zero or more, but not '/'
                    j = k
                    while j <= len(seg_str):
                        if rec(i + 1, j):
                            result = True
                            break
                        if j < len(seg_str) and seg_str[j] == '/':
                            break
                        j += 1
                elif tok[0] == 'starstar':
                    # consume the '**' segments
                    count = tok[1]
                    # number of '/' to consume = count - 1
                    need = count - 1
                    if k + need <= len(seg_str):
                        if all(seg_str[m] == '/' for m in range(k, k + need)):
                            result = rec(i + 1, k + need)
            memo[(i, k)] = result
            return result

        memo = {}
        return rec(0, 0)

    for seg_pat, seg_path in zip(pat_segments, segments):
        seg_tokens = parse_segment(seg_pat)
        if not match_segments(seg_tokens, seg_path):
            return False
    return True