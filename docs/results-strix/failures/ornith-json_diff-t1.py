def json_diff(a, b):
    if isinstance(a, dict) and isinstance(b, dict):
        keys = set(a) | set(b)
        for key in keys:
            path = key if not key else key + "."
            if key not in a:
                yield from json_diff(None, b[key], prefix=path)
            elif key not in b:
                yield from json_diff(a[key], None, prefix=path)
            else:
                yield from json_diff(a[key], b[key], prefix=path)
    else:
        if a != b:
            yield ...