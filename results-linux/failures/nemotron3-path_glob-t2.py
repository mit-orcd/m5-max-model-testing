import re

def match(pattern, path):
    def parse_pattern(p):
        tokens = []
        i = 0
        while i < len(p):
            if p[i] == '\\':
                i += 1
                if i < len(p):
                    tokens.append(('LIT', re.escape(p[i])))
                i += 1
            elif p[i] == '*':
                if i + 1 < len(p) and p[i+1] == '*':
                    tokens.append(('DBLSTAR', None))
                    i += 2
                else:
                    tokens.append(('STAR', None))
                    i += 1
            elif p[i] == '?':
                tokens.append(('QM', None))
                i += 1
            elif p[i] == '[':
                j = i + 1
                if j < len(p) and p[j] == '!':
                    j += 1
                    negate = True
                else:
                    negate = False
                chars = []
                while j < len(p) and p[j] != ']':
                    if j + 2 < len(p) and p[j+1] == '-':
                        chars.append((p[j], p[j+2]))
                        j += 3
                    else:
                        chars.append(p[j])
                        j += 1
                if j >= len(p) or p[j] != ']':
                    raise ValueError("Unclosed bracket")
                tokens.append(('CLASS', (chars, negate)))
                i = j + 1
            else:
                tokens.append(('LIT', re.escape(p[i])))
                i += 1
        return tokens

    def compile_regex(tokens):
        parts = ['^']
        i = 0
        while i < len(tokens):
            t = tokens[i]
            if t[0] == 'LIT':
                parts.append(t[1])
            elif t[0] == 'QM':
                parts.append('[^/]')
            elif t[0] == 'STAR':
                parts.append('[^/]*')
            elif t[0] == 'DBLSTAR':
                parts.append('(?:[^/]+/)*')
            elif t[0] == 'CLASS':
                chars, negate = t[1]
                if negate:
                    cls = '[^'
                else:
                    cls = '['
                for c in chars:
                    if isinstance(c, tuple):
                        cls += re.escape(c[0]) + '-' + re.escape(c[1])
                    else:
                        cls += re.escape(c)
                cls += ']'
                parts.append(cls)
            i += 1
        parts.append('$')
        return ''.join(parts)

    tokens = parse_pattern(pattern)
    regex = compile_regex(tokens)
    return re.fullmatch(regex, path) is not None