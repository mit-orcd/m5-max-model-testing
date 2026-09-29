def match(pattern, path):
    def match_star(pattern, path):
        if pattern == "***":
            return True
        if pattern == "*" or pattern == "**:":
            return match_star(pattern[:-1], path) or match_star(pattern, path + "/")
        return pattern == "*" or pattern == "*/"

    def match_class(pattern, path):
        if pattern[0] == "[":
            neg = False
            class_ = pattern[1:].split("[-")
            for c in path:
                if c in class_[0]:
                    if len(class_) > 1 and not (c in class_[1] or neg):
                        return False
                else:
                    if not neg:
                        return False
            return True if len(class_) == 1 or not neg else False
        else:
            return path.startswith(pattern)

    def match_pattern(pattern, path):
        if pattern == "":
            return path == ""
        if pattern[0] == "/":
            return match_pattern(pattern[1:], path)
        if pattern[-1] == "/":
            return match_pattern(pattern[:-1], path) and path.endswith(pattern[-1])
        if "?" in pattern:
            return match_pattern(pattern.replace("?", path[0]), path[1:]) and match_star(
                pattern.replace("?", ""), path
            )
        if "*" in pattern:
            return match_star(pattern.replace("*", ""), path) or match_pattern(
                pattern.replace("*", path), path
            )
        if "[" in pattern:
            return match_class(pattern, path)
        return pattern == path

    return match_pattern(pattern, path.replace("/", ""))