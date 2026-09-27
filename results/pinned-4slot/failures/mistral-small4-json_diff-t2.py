def json_diff(a, b, path=""):
    diffs = []

    if type(a) != type(b):
        diffs.append(path)
        return diffs

    if isinstance(a, dict):
        all_keys = set(a.keys()) | set(b.keys())
        for key in sorted(all_keys):
            new_path = f"{path}.{key}" if path else key
            if key not in a:
                diffs.append(f"{new_path} (missing in a)")
            elif key not in b:
                diffs.append(f"{new_path} (missing in b)")
            else:
                diffs.extend(json_diff(a[key], b[key], new_path))
    elif isinstance(a, list):
        if a != b:
            diffs.append(path)
    else:
        if a != b:
            diffs.append(path)

    return diffs