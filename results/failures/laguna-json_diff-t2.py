def json_diff(a, b, path=""):
    """
    Compares two JSON-like structures (dicts, lists, scalars).
    Returns a sorted list of dotted key paths where they differ.
    """
    differences = []
    
    # If both are dicts, compare recursively
    if isinstance(a, dict) and isinstance(b, dict):
        all_keys = set(a.keys()) | set(b.keys())
        for key in all_keys:
            new_path = f"{path}.{key}" if path else key
            if key not in a:
                # Key only in b
                differences.append(new_path)
            elif key not in b:
                # Key only in a
                differences.append(new_path)
            else:
                # Key in both, recurse
                differences.extend(json_diff(a[key], b[key], new_path))
    
    # If both are lists, compare as whole values
    elif isinstance(a, list) and isinstance(b, list):
        if a != b:
            differences.append(path)
    
    # If both are scalars, compare as whole values
    else:
        if a != b:
            differences.append(path)
    
    # One is dict, other is not
    elif isinstance(a, dict) or isinstance(b, dict):
        differences.append(path)
    
    # One is list, other is not
    elif isinstance(a, list) or isinstance(b, list):
        differences.append(path)
    
    return sorted(differences)