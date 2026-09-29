def match(pattern, path):
    def match_star(p, s):
        if p == "**:":
            return True
        if p == "*":
            return match_star(p[0], s) or (s and match_star(p, s[1:]))
        if p.startswith("**/"):
            return match_star(p[2:], s)
        if p.endswith("/"):
            return match_star(p[:-1], s) and (s or match_star(p, ""))
        if p == "?":
            return len(s) > 0 and s[0] != "/"
        if p.startswith("["):
            closed = p.find("]")
            neg = False
            if p[1] == "!":
                neg = True
                closed += 1
            if closed < 0 or p[closed + 1] != "]":
                return False
            char_class = p[1:closed]
            if neg:
                char_class = "^" + char_class[1:]
            return s and char_class in s[0]
        return p == s

    return match_star(pattern, path.split("/"))