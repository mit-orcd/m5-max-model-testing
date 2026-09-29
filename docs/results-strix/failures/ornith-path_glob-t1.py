def match(pattern, path):
    # Tokenize the pattern into a list of items.
    # Each item is one of:
    #   ('literal', ch)         - a single literal character
    #   ('literal', s)          - a run of literal characters (s may be empty)
    #   ('question',)           - '?' matches exactly one non-slash character
    #   ('star',)               - '*' matches zero or more non-slash characters
    #   ('starstar',)           - '**' matches zero or more whole segments
    #   ('class', neg, chars)   - a character class; neg is bool; chars is a set of literal chars
    #   ('rangeclass', neg, ranges) - ranges is a list of (lo, hi) tuples
    def tokenize(pat):
        tokens = []
        i = 0
        n = len(pat)
        while i < n:
            c = pat[i]
            if c == '\\':
                if i + 1 < n:
                    tokens.append(('literal', pat[i + 1]))
                    i += 2
                else:
                    tokens.append(('literal', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('question',))
                i += 1
            elif c == '*':
                if i + 1 < n and pat[i + 1] == '*':
                    tokens.append(('starstar',))
                    i += 2
                    # collapse a trailing '/' after '**'
                    if i < n and pat[i] == '/':
                        i += 1
                else:
                    tokens.append(('star',))
                    i += 1
            elif c == '[':
                j = i + 1
                neg = False
                if j < n and pat[j] == '!':
                    neg = True
                    j += 1
                # handle a leading ']' as a literal bracket
                if j < n and pat[j] == ']':
                    j += 1
                chars = set()
                ranges = []
                while j < n and pat[j] != ']':
                    if pat[j] == '\\' and j + 1 < n:
                        ch = pat[j + 1]
                        chars.add(ch)
                        ranges.append((ch, ch))
                        j += 2
                        continue
                    # detect a range like a-z
                    if j + 2 < n and pat[j + 1] == '-' and pat[j + 2] != ']':
                        lo = pat[j]
                        hi = pat[j + 2]
                        if hi >= lo:
                            ranges.append((lo, hi))
                            chars.update(chr(k) for k in range(ord(lo), ord(hi) + 1))
                            j += 3
                        else:
                            chars.add(lo)
                            ranges.append((lo, lo))
                            j += 1
                    else:
                        chars.add(pat[j])
                        ranges.append((pat[j], pat[j]))
                        j += 1
                if j < n and pat[j] == ']':
                    tokens.append(('class', neg, chars, ranges))
                    i = j + 1
                else:
                    # unterminated class; treat '[' literally
                    tokens.append(('literal', '['))
                    i += 1
            else:
                # accumulate a run of literal characters
                start = i
                while i < n:
                    cc = pat[i]
                    if cc in '\\?*[':
                        break
                    if cc == '/' and not (tokens and tokens[-1][0] == 'literal'):
                        break
                    i += 1
                tokens.append(('literal', pat[start:i]))
        return tokens

    toks = tokenize(pattern)

    # Precompute for '**' collapsing: a '**' that is adjacent to '/' on
    # either side (or at either end) can be merged with surrounding
    # slashes. We'll handle merging during matching via the recursive
    # function below.

    def char_in_class(neg, chars, ranges, ch):
        if ch == '/':
            return False
        if ch in chars:
            return True
        for lo, hi in ranges:
            if lo <= ch <= hi:
                return True
        return not neg

    def match_tokens(toks, pi, s):
        # Match toks[pi:] against the whole of s.
        while pi < len(toks):
            tok = toks[pi]
            kind = tok[0]

            if kind == 'literal':
                s_ = tok[1]
                if not s.startswith(s_):
                    return False
                s = s[len(s_):]
                pi += 1
            elif kind == 'question':
                if not s or s[0] == '/':
                    return False
                s = s[1:]
                pi += 1
            elif kind == 'star':
                # '*' matches zero or more non-slash chars.
                # Try to match the rest starting at each position
                # until we hit a '/'.
                rest = toks[pi + 1:]
                k = 0
                while k < len(s) and s[k] != '/':
                    if match_tokens(rest, 0, s[k:]):
                        return True
                    k += 1
                # also try matching rest against empty at this position
                if match_tokens(rest, 0, s[k:]):
                    return True
                return False
            elif kind == 'starstar':
                # '**' matches zero or more whole segments.
                rest = toks[pi + 1:]
                # Try matching rest at every segment boundary in s.
                if match_tokens(rest, 0, s):
                    return True
                k = 0
                while k < len(s):
                    if s[k] == '/':
                        if match_tokens(rest, 0, s[k + 1:]):
                            return True
                        k += 1
                    else:
                        k += 1
                        while k < len(s) and s[k] != '/':
                            k += 1
                return False
            elif kind in ('class', 'rangeclass'):
                neg, chars, ranges = tok[1], tok[2], tok[3]
                if not s or not char_in_class(neg, chars, ranges, s[0]):
                    return False
                s = s[1:]
                pi += 1
            else:
                return False

        return s == ''

    return match_tokens(toks, 0, path)