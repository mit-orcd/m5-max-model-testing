def match(pattern, path):
    def is_match(p, s, pi, si):
        while pi < len(p) and si < len(s):
            if p[pi] == '*':
                pi += 1
                if pi == len(p):
                    return True
                while si < len(s) and s[si] != '/':
                    if is_match(p, s, pi, si):
                        return True
                    si += 1
                if si == len(s):
                    return is_match(p, s, pi, si)
            elif p[pi] == '?':
                if s[si] == '/':
                    return False
                pi += 1
                si += 1
            elif p[pi] == '[':
                pi += 1
                neg = p[pi] == '!'
                if neg:
                    pi += 1
                end = p.find(']', pi)
                if end == -1:
                    return False
                char_class = p[pi:end]
                if s[si] == '/':
                    return False
                match_char = s[si] in char_class
                if neg:
                    match_char = not match_char
                if not match_char:
                    return False
                pi = end + 1
                si += 1
            elif p[pi] == '\\':
                pi += 1
                if pi >= len(p):
                    return False
                if p[pi] != s[si]:
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
    
    return is_match(pattern, path, 0, 0)