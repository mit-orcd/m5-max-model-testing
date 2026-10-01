def json_diff(a, b, prefix=""):
    """
    Compares two JSON-like structures (dicts, lists, scalars).
    Returns a sorted list of dotted key paths where they differ.
    """
    diffs = []
    
    # If both are dicts, recurse
    if isinstance(a, dict) and isinstance(b, dict):
        all_keys = set(a.keys()) | set(b.keys())
        for key in all_keys:
            path = f"{prefix}.{key}" if prefix else key
            if key not in a:
                diffs.append(f"{path} (missing in left)")
            elif key not in b:
                diffs.append(f"{path} (missing in right)")
            else:
                diffs.extend(json_diff(a[key], b[key], path))
    else:
        # For lists and scalars, compare as whole values
        if a != b:
            if prefix:
                diffs.append(prefix)
            else:
                diffs.append("(root)")
    
    return sorted(diffs)