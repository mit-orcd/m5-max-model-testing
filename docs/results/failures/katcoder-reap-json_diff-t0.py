def json_diff(a, b, prefix=""):
    diffs = []
    if isinstance(a, dict) and isinstance(b, dict):
        all_keys = sorted(set(a.keys()) | set(b.keys()))
        for key in all_keys:
            path = f"{prefix}.{key}" if prefix else key
            if key not in a:
                diffs.append(f"{path} (missing in a)")
            elif key not in b:
                diffs.append(f"{path} (missing in b)")
            else:
                diffs.extend(json_diff(a[key], b[key], path))
    else:
        if a != b:
            diffs.append(prefix)
    return diffs