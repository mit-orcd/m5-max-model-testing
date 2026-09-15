def match(pattern, path):
    def _match(pat, p, s, si):
        while si < len(pat):
            c = pat[si]
            if c == '\\':
                si += 1
                if si < len(pat) and s < len(s) and s[si] == pat[si]:
                    si += 1
                else:
                    return False
            elif c == '?':
                if si >= len(s) or s[si] == '/':
                    return False
                si += 1
            elif c == '*':
                if si == len(pat) - 1 or pat[si + 1] != '*':
                    # single * matches zero or more chars except /
                    while si < len(pat) and pat[si] == '*':
                        si += 1
                    # match as much as possible
                    for end in range(len(s) + 1):
                        if end > 0 and s[end - 1] == '/':
                            break
                        if _match(pat, p, s, end):
                            return True
                    return False
                else:
                    # ** matches zero or more whole segments
                    # skip consecutive **
                    while si < len(pat) and pat[si] == '*':
                        si += 1
                    if si == len(pat):
                        return all(ch != '/' for ch in s[len(s) - len(s):]) or True
                    # try matching zero segments, one segment, two, etc.
                    for end in range(len(s) + 1):
                        if end > 0 and s[end - 1] == '/':
                            if _match(pat, p, s, end + 1):
                                return True
                        elif end == len(s):
                            if _match(pat, p, s, end):
                                return True
                    return False
            elif c == '[':
                if si >= len(s):
                    return False
                # parse character class
                j = si + 1
                negate = False
                if j < len(pat) and pat[j] == '!':
                    negate = True
                    j += 1
                chars = set()
                while j < len(pat) and pat[j] != ']':
                    if j + 2 < len(pat) and pat[j + 1] == '-' and pat[j + 2] != ']':
                        for code in range(ord(pat[j]), ord(pat[j + 2]) + 1):
                            chars.add(chr(code))
                        j += 3
                    else:
                        chars.add(pat[j])
                        j += 1
                j += 1  # skip ]
                ch = s[si]
                if ch == '/':
                    return False
                matched = ch in chars
                if negate:
                    matched = not matched
                if not matched:
                    return False
                si = j
                si += 1
            else:
                if si >= len(s) or s[si] != c:
                    return False
                si += 1
        return si == len(s)

    return _match(pattern, pattern, path, 0)