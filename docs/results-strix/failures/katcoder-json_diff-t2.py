def json_diff(a, b, prefix=""):
    diffs = []
    if isinstance(a, dict) and isinstance(b, dict):
        all_keys = sorted(set(a.keys()) | set(b.keys()))
        for key in all_keys:
            path = f"{prefix}.{key}" if prefix else key
            if key not in a:
                diffs.append(path + " (removed)")
            elif key not in b:
                diffs.append(path + " (added)")
            else:
                diffs.extend(json_diff(a[key], b[key], path))
    elif isinstance(a, list) and isinstance(b, list):
        if a != b:
            diffs.append(prefix)
    else:
        if a != b:
            diffs.append(prefix)
    return diffs