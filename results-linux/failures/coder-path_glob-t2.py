def match(pattern, path):
    def is_match(p, s, pi, si):
        while pi < len(p) and si < len(s):
            if p[pi] == '*':
                pi += 1
                if pi == len(p):
                    return True
                while pi < len(p) and p[pi] == '*':
                    pi += 1
                if pi == len(p):
                    return True
                while si < len(s):
                    if is_match(p, s, pi, si):
                        return True
                    if s[si] == '/':
                        break
                    si += 1
                return False
            elif p[pi] == '?':
                if s[si] == '/':
                    return False
                pi += 1
                si += 1
            elif p[pi] == '[':
                pi += 1
                neg = False
                if p[pi] == '!':
                    neg = True
                    pi += 1
                char_class = []
                while pi < len(p) and p[pi] != ']':
                    if pi + 2 < len(p) and p[pi + 1] == '-' and p[pi + 2] != ']':
                        char_class.extend(chr(c) for c in range(ord(p[pi]), ord(p[pi + 2]) + 1))
                        pi += 3
                    else:
                        char_class.append(p[pi])
                        pi += 1
                if pi < len(p) and p[pi] == ']':
                    pi += 1
                if si >= len(s) or (s[si] in char_class) == (not neg):
                    return False
                si += 1
            elif p[pi] == '\\':
                pi += 1
                if pi >= len(p):
                    return False
                if p[pi] != s[si]:
                    return False
                pi += 1
                si += 1
            elif p[pi] == '/':
                if s[si] != '/':
                    return False
                pi += 1
                si += 1
            else:
                if p[pi] != s[si]:
                    return False
                pi += 1
                si += 1
        
        while pi < len(p) and p[pi] == '*':
            pi += 1
        
        return pi == len(p) and si == len(s)
    
    if not pattern or not path:
        return pattern == path
    
    return is_match(pattern, path, 0, 0)