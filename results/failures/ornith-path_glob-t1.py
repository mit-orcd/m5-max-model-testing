def match(pattern, path):
    def parse_class(seg, i):
        i += 1
        negate = False
        if i < len(seg) and seg[i] == '!':
            negate = True
            i += 1
        chars = set()
        last = None
        while i < len(seg) and seg[i] != ']':
            c = seg[i]
            if last is not None and c == ']' and i + 1 < len(seg):
                chars.add(']')
                i += 1
                last = None
                continue
            if c == '-' and last is not None and i + 1 < len(seg) and seg[i + 1] != ']':
                lo = last
                hi = seg[i + 1]
                for code in range(ord(lo), ord(hi) + 1):
                    chars.add(chr(code))
                last = None
                i += 2
                continue
            chars.add(c)
            last = c
            i += 1
        if negate:
            def f(c):
                return c != '/' and c not in chars
        else:
            def f(c):
                return c in chars
        return f, i

    def parse_segment(seg):
        items = []
        i = 0
        n = len(seg)
        while i < n:
            c = seg[i]
            if c == '\\':
                if i + 1 < n:
                    items.append(('char', seg[i + 1]))
                    i += 2
                else:
                    items.append(('char', '\\'))
                    i += 1
            elif c == '?':
                items.append(('qmark',))
                i += 1
            elif c == '*':
                items.append(('star',))
                i += 1
            elif c == '[':
                f, ni = parse_class(seg, i)
                items.append(('class', f))
                i = ni
            else:
                items.append(('char', c))
                i += 1
        return items

    segments = pattern.split('/')
    paths = path.split('/')

    def recurse(pi, si):
        if pi == len(segments):
            return si == len(paths)
        seg = segments[pi]
        if seg == '**':
            for k in range(si, len(paths) + 1):
                if recurse(pi + 1, k):
                    return True
            return False
        if si >= len(paths):
            return False
        return match_segment(parse_segment(seg), paths[si])

    def match_segment(items, text):
        i = 0
        j = 0
        star = -1
        star_j = -1
        n = len(items)
        m = len(text)
        while True:
            while i < n:
                t = items[i]
                if t[0] == 'char':
                    if j < m and t[1] == text[j]:
                        i += 1
                        j += 1
                    else:
                        return False
                elif t[0] == 'qmark':
                    if j < m and text[j] != '/':
                        i += 1
                        j += 1
                    else:
                        return False
                elif t[0] == 'class':
                    if j < m and t[1](text[j]):
                        i += 1
                        j += 1
                    else:
                        return False
                elif t[0] == 'star':
                    i += 1
                    star = i
                    star_j = j
                    break
                else:
                    return False
            if i >= n:
                return j == m
            if star != -1:
                j = star_j + 1
                star_j += 1
                i = star
            else:
                return False

    return recurse(0, 0)