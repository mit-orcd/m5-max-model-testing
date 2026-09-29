def match(pattern, path):
    def match_segment(pattern_seg, path_seg):
        if pattern_seg == "*":
            return True
        if pattern_seg == "**":
            return match_segment(pattern_seg, path_seg[1:]) or (
                path_seg and match_segment(pattern_seg, path_seg[0] + path_seg[1:])
            )
        if pattern_seg.startswith("["):
            closed = pattern_seg.find("]")
            negated = closed > 0 and pattern_seg[closed - 1] == "!"
            char_class = pattern_seg[1:closed if negated else closed]
            return negated ^ (path_seg in char_class)
        return path_seg == pattern_seg

    pattern_parts, path_parts = pattern.split("/"), path.split("/")
    return len(pattern_parts) == len(path_parts) and all(
        match_segment(pattern_seg, path_seg) for pattern_seg, path_seg in zip(pattern_parts, path_parts)
    )