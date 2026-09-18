def match(pattern, path):
    def compile(pat):
        out = []
        i = 0
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
                    i += 2
                else:
                    out.append(('star1',))
                    i += 1
            elif c == '[':
                j = i + 1
                negate = False
                if j < len(pat) and pat[j] == '!':
                    negate = True
                    j += 1
                chars = set()
                ranges = []
                while j < len(pat) and pat[j] != ']':
                    nc = pat[j]
                    if j + 2 < len(pat) and pat[j + 1] == '-':
                        lo = nc
                        hi = pat[j + 2]
                        if lo <= hi:
                            ranges.append((lo, hi))
                        j += 2
                    else:
                        chars.add(nc)
                    j += 1
                if j < len(pat) and pat[j] == ']':
                    j += 1
                i = j
                out.append(('class', negate, chars, ranges))
            else:
                out.append(('lit', c))
                i += 1
        return out

    prog = compile(pattern)
    segs = path.split('/')

    def match_class(ch, negate, chars, ranges):
        if ch == '/':
            return False
        if ch in chars:
            return not negate
        for lo, hi in ranges:
            if lo <= ch <= hi:
                return not negate
        return negate

    memo = {}

    def dp(i, j):
        key = (i, j)
        if key in memo:
            return memo[key]
        if i == len(prog) and j == len(segs):
            result = True
        elif i < len(prog) and prog[i][0] == 'star':
            result = dp(i + 1, j) or (j < len(segs) and dp(i, j + 1))
        elif i < len(prog) and prog[i][0] == 'star1':
            result = dp(i + 1, j) or (j < len(segs) and dp(i, j + 1) and segs[j] != '')
        elif i < len(prog) and j < len(segs) and prog[i][0] == 'one':
            result = dp(i + 1, j + 1)
        elif i < len(prog) and j < len(segs) and prog[i][0] == 'lit' and prog[i][1] == segs[j]:
            result = dp(i + 1, j + 1)
        elif i < len(prog) and j < len(segs) and prog[i][0] == 'class':
            _, negate, chars, ranges = prog[i]
            if match_class(segs[j], negate, chars, ranges):
                result = dp(i + 1, j + 1)
            else:
                result = False
        else:
            result = False
        memo[key] = result
        return result

    return dp(0, 0)