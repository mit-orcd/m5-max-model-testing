def json_diff(a, b):
    def diff_path(path, a, b):
        if path in b_paths:
            return
        b_paths.add(path)
        if isinstance(a, dict) and isinstance(b, dict):
            for k in set(a.keys()) | set(b.keys()):
                diff_path(path + "." + k, a.get(k), b.get(k))
        elif isinstance(a, list) and isinstance(b, list):
            for i, (x, y) in enumerate(zip(a, b)):
                diff_path(path + "[" + str(i) + "]", x, y)
        else:
            b_paths.add(path) if a != b else None

    b_paths = set()
    diff_path("", a, b)
    return list(b_paths)