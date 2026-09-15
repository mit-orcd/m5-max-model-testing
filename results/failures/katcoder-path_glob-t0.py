def match(pattern, path):
    def _match(pat, p, s, st):
        while p < len(pat) and st < len(s):
            c = pat[p]
            if c == '\\':
                p += 1
                if p >= len(pat):
                    return False
                if s[st] != pat[p]:
                    return False
                p += 1
                st += 1
            elif c == '?':
                if s[st] == '/':
                    return False
                p += 1
                st += 1
            elif c == '*':
                if p + 1 < len(pat) and pat[p + 1] == '*':
                    # **
                    p += 2
                    # skip trailing /
                    if p < len(pat) and pat[p] == '/':
                        p += 1
                    # try matching zero or more segments
                    for i in range(st, len(s) + 1):
                        if _match(pat, p, s, i):
                            return True
                    return False
                else:
                    # single *
                    p += 1
                    for i in range(st, len(s) + 1):
                        if s[i - 1] == '/':
                            break
                        if _match(pat, p, s, i):
                            return True
                    return False
            elif c == '[':
                # parse character class
                q = p + 1
                negate = False
                if q < len(pat) and pat[q] == '!':
                    negate = True
                    q += 1
                chars = set()
                while q < len(pat) and pat[q] != ']':
                    if q + 2 < len(pat) and pat[q + 1] == '-' and pat[q + 2] != ']':
                        for code in range(ord(pat[q]), ord(pat[q + 2]) + 1):
                            chars.add(chr(code))
                        q += 3
                    else:
                        chars.add(pat[q])
                        q += 1
                q += 1  # skip ]
                if s[st] not in chars:
                    if negate:
                        pass
                    else:
                        return False
                if negate and s[st] in chars:
                    return False
                p = q
                st += 1
            else:
                if s[st] != c:
                    return False
                p += 1
                st += 1
        # consume remaining * in pattern
        while p < len(pat) and pat[p] == '*':
            if p + 1 < len(pat) and pat[p + 1] == '*':
                p += 2
                if p < len(pat) and pat[p] == '/':
                    p += 1
            else:
                p += 1
        return p == len(pat) and st == len(s)
    return _match(pattern, 0, path, 0)