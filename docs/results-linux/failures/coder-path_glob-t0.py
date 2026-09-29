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
                end = p.find(']', pi)
                if end == -1:
                    return False
                char_class = p[pi+1:end]
                if si >= len(s) or s[si] == '/':
                    return False
                if char_class.startswith('!'):
                    if s[si] in char_class[1:]:
                        return False
                else:
                    if s[si] not in char_class:
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