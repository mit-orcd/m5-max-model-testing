def json_diff(a, b):
    def is_scalar(v):
        return isinstance(v, (int, float, str, bool)) or v is None

    differences = []

    if isinstance(a, dict) and isinstance(b, dict):
        all_keys = set(a.keys()) | set(b.keys())
        for key in sorted(all_keys):
            if key not in a:
                differences.append(key)
            elif key not in b:
                differences.append(key)
            else:
                sub_diffs = json_diff(a[key], b[key])
                for d in sub_diffs:
                    differences.append(f"{key}.{d}")
    elif isinstance(a, list) and isinstance(b, list):
        if a != b:
            differences.append("")
    elif is_scalar(a) and is_scalar(b):
        if a != b:
            differences.append("")
    else:
        if a != b:
            differences.append("")

    return sorted(differences)