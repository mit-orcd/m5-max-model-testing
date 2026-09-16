def json_diff(a, b, path=""):
    diffs = []
    if type(a) != type(b):
        diffs.append(path)
        return diffs
    if isinstance(a, dict):
        all_keys = sorted(set(a.keys()) | set(b.keys()))
        for k in all_keys:
            current_path = f"{path}.{k}" if path else k
            if k not in a:
                diffs.append(f"{current_path} (missing in a)")
            elif k not in b:
                diffs.append(f"{current_path} (missing in b)")
            else:
                diffs.extend(json_diff(a[k], b[k], current_path))
    elif isinstance(a, list):
        if a != b:
            diffs.append(path)
    else:
        if a != b:
            diffs.append(path)
    return diffs