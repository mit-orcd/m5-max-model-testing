def match(pattern, path):
    def match_segment(pseg, p, s):
        if pseg == "**":
            return True
        if pseg == "?" and len(s) > 0:
            return match_segment(p, p[1:], s[1:])
        if pseg == "*" and len(s) > 0:
            return match_segment(p, p[1:], s) or match_segment(p, p, s[1:])
        if pseg == "[" and s[0] in p:
            return match_segment(p, p[1:], s[1:])
        if pseg == "[" and s[0] == "-":
            return match_segment(p, p[1:], s[1:])
        if pseg == "[" and s[0] == "]":
            return False
        return pseg == s

    return match_segment(pattern, pattern, path.split("/")) and path.endswith("/")