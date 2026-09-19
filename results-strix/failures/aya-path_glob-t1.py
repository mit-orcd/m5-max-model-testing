def match(pattern, path):
    def match_segment(pattern_segment, path_segment):
        if pattern_segment == "*":
            return True
        if pattern_segment == "**":
            return match_segment(pattern_segment, path_segment) or match(
                pattern, path[:path.index(path_segment)] + path[path.index(path_segment) + 1 :]
            )
        if pattern_segment.startswith("["):
            return (
                path_segment in pattern_segment[1:-1]
                if pattern_segment[1] != "!"
                else not path_segment in pattern_segment[1:-1]
            )
        if pattern_segment.startswith("\["):
            return path_segment in pattern_segment[1:-1]
        return pattern_segment == path_segment or (
            pattern_segment.startswith("/") and path_segment == ""
        )

    pattern_segments = pattern.split("/")
    path_segments = path.split("/")
    return match_segment(pattern_segments[0], path_segments[0]) and all(
        match_segment(p, pp) for p, pp in zip(pattern_segments[1:], path_segments[1:])
    )