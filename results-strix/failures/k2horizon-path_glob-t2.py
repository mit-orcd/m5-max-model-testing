import re

def match(pattern, path):
    # Compile pattern to regex
    i = 0
    n = len(pattern)
    tokens = []  # list of ('seg', regex) or ('globstar',)
    
    def parse_segment(s):
        out = []
        j = 0
        m = len(s)
        while j < m:
            c = s[j]
            if c == '\\' and j + 1 < m:
                out.append(re.escape(s[j+1]))
                j += 2
                continue
            if c == '[':
                # parse character class
                k = j + 1
                if k < m and s[k] == '!':
                    k += 1
                neg = k > j + 1
                chars = []
                if k < m and s[k] == ']':
                    chars.append('\\]')
                    k += 1
                while k < m and s[k] != ']':
                    if s[k] == '\\' and k + 1 < m:
                        chars.append(re.escape(s[k+1]))
                        k += 2
                        continue
                    if k + 2 < m and s[k+1] == '-' and s[k+2] != ']':
                        chars.append(re.escape(s[k]) + '-' + re.escape(s[k+2]))
                        k += 3
                        continue
                    chars.append(re.escape(s[k]))
                    k += 1
                if k < m and s[k] == ']':
                    k += 1
                else:
                    # unclosed, treat as literal
                    chars.append(re.escape('['))
                    k = j + 1
                cls = '[' + ('^' if neg else '') + ''.join(chars) + ']'
                out.append(cls)
                j = k
                continue
            if c == '*':
                out.append('[^/]*')
                j += 1
                continue
            if c == '?':
                out.append('[^/]')
                j += 1
                continue
            out.append(re.escape(c))
            j += 1
        return ''.join(out)
    
    # Split pattern into segments respecting **
    # We need to handle '**' as a whole segment
    segs = []
    j = 0
    m = n
    while j < m:
        if pattern[j:j+2] == '**':
            # check it's a whole segment: next char is end or '/'
            if j + 2 == m or pattern[j+2] == '/':
                segs.append('**')
                j += 2
                if j < m and pattern[j] == '/':
                    j += 1
                continue
        # otherwise consume until next '/'
        k = j
        while k < m and pattern[k] != '/':
            k += 1
        segs.append(pattern[j:k])
        j = k
        if j < m and pattern[j] == '/':
            j += 1
    
    # Build regex from segments
    parts = []
    for seg in segs:
        if seg == '**':
            parts.append('(?:[^/]+/)*')
        else:
            parts.append('(?:' + parse_segment(seg) + ')')
    regex = '^' + '/'.join(parts) + '$'
    return re.match(regex, path) is not None