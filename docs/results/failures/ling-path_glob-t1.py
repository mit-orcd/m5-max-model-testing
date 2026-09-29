def match(pattern, path):
    def compile_pattern(pat):
        i = 0
        out = []
        while i < len(pat):
            c = pat[i]
            if c == '\\' and i + 1 < len(pat):
                out.append(('lit', pat[i + 1]))
                i += 2
            elif c == '?':
                out.append(('one',))
                i += 1
            elif c == '*':
                if i + 1 < len(pat) and pat[i + 1] == '*':
                    out.append(('star',))
                    i += 1
                else:
                    out.append(('star1',))
                i += 1
            elif c == '[':
                j = i + 1
                if j >= len(pat):
                    out.append(('lit', '['))
                else:
                    negate = False
                    if pat[j] == '!':
                        negate = True
                        j += 1
                    chars = set()
                    ranges = []
                    while j < len(pat) and pat[j] != ']':
                        if j + 2 < len(pat) and pat[j + 1] == '-':
                            ranges.append((pat[j], pat[j + 2]))
                            j += 3
                        else:
                            chars.add(pat[j])
                            j += 1
                    if j < len(pat) and pat[j] == ']':
                        out.append(('class', chars, ranges, negate))
                        i = j + 1
                    else:
                        out.append(('lit', '['))
                i += 1
            else:
                out.append(('lit', c))
                i += 1
        return out

    prog = compile_pattern(pattern)
    segs = path.split('/')

    def match_char(ch, cls):
        if cls is None:
            return False
        if ch in cls[0]:
            return True
        for lo, hi in cls[1]:
            if lo <= ch <= hi:
                return True
        return cls[2] and not any((lo <= ch <= hi for lo, hi in cls[1]))

    memo = {}

    def dp(pi, si):
        if (pi, si) in memo:
            return memo[(pi, si)]
        if pi == len(prog):
            result = si == len(segs)
        else:
            kind = prog[pi][0]
            if kind == 'lit':
                if si >= len(segs):
                    result = False
                else:
                    result = (prog[pi][1] == segs[si] and dp(pi + 1, si + 1))
            elif kind == 'one':
                if si >= len(segs):
                    result = False
                else:
                    result = (len(segs[si]) == 1 and dp(pi + 1, si + 1))
            elif kind == 'star1':
                if si < len(segs) and '/' not in segs[si]:
                    result = dp(pi + 1, si) or dp(pi, si + 1)
                else:
                    result = dp(pi + 1, si)
            elif kind == 'star':
                result = dp(pi + 1, si) or (si < len(segs) and dp(pi, si + 1))
            elif kind == 'class':
                if si >= len(segs):
                    result = False
                else:
                    ok = False
                    for ch in segs[si]:
                        if match_char(ch, prog[pi][1:]):
                            ok = True
                            break
                    result = ok and dp(pi + 1, si + 1)
            else:
                result = False
        memo[(pi, si)] = result
        return result

    return dp(0, 0)