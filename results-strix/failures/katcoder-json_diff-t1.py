def json_diff(a, b, prefix=""):
    diffs = []
    if isinstance(a, dict) and isinstance(b, dict):
        all_keys = set(a.keys()) | set(b.keys())
        for key in all_keys:
            path = f"{prefix}.{key}" if prefix else key
            if key not in a:
                diffs.append(f"{path} <deleted>")
            elif key not in b:
                diffs.append(f"{path} <added>")
            else:
                diffs.extend(json_diff(a[key], b[key], path))
    elif isinstance(a, list) and isinstance(b, list):
        if a != b:
            diffs.append(f"{prefix} <changed>")
    elif a != b:
        diffs.append(f"{prefix} <changed>")
    return sorted(diffs)