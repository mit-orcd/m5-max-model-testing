def json_diff(a, b, path=""):
    """
    Compares two JSON-like structures and returns a sorted list of dotted key paths where they differ.
    """
    diffs = []

    if type(a) != type(b):
        diffs.append(path if path else "<root>")
        return diffs

    if isinstance(a, dict):
        keys = set(a.keys()) | set(b.keys())
        for key in keys:
            current_path = f"{path}.{key}" if path else key
            diffs.extend(json_diff(a.get(key), b.get(key), current_path))
    elif isinstance(a, list):
        max_len = max(len(a), len(b))
        for i in range(max_len):
            current_path = f"{path}[{i}]"
            if i < len(a) and i < len(b):
                diffs.extend(json_diff(a[i], b[i], current_path))
            elif i < len(a):
                diffs.append(current_path)
            else:
                diffs.append(current_path)
    else:
        if a != b:
            diffs.append(path if path else "<root>")

    return sorted(diffs)