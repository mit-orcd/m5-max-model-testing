def json_diff(a, b, path=""):
    diffs = []

    if a == b:
        return diffs

    if isinstance(a, dict) and isinstance(b, dict):
        all_keys = set(a.keys()) | set(b.keys())
        for key in sorted(all_keys):
            new_path = f"{path}.{key}" if path else key
            if key not in a:
                diffs.append(f"{new_path} (missing in a)")
            elif key not in b:
                diffs.append(f"{new_path} (missing in b)")
            else:
                diffs.extend(json_diff(a[key], b[key], new_path))
    else:
        diffs.append(path)

    return diffs