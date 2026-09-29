def json_diff(a, b, _prefix=""):
    if isinstance(a, dict) and isinstance(b, dict):
        diffs = []
        for key in set(a) | set(b):
            if key not in a:
                diffs.append(f"{_prefix}{key}")
            elif key not in b:
                diffs.append(f"{_prefix}{key}")
            else:
                diffs.extend(json_diff(a[key], b[key], f"{_prefix}{key}."))
        return diffs
    if a != b:
        return [_prefix]
    return []