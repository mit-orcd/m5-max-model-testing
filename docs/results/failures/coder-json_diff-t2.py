def json_diff(a, b, path=""):
    """
    Compare two JSON-like structures and return a sorted list of differing paths.
    
    Args:
        a: First JSON-like structure (dict, list, or scalar)
        b: Second JSON-like    b: Second JSON-like structure (dict, list, or scalar)
        path: Current path in the structure (used for recursion)
    
    Returns:
        A sorted list of string paths where values differ between a and b.
    """
    if type(a) != type(b):
        return [path] if path else [""]

    if isinstance(a, dict):
        all_keys = set(a.keys()) | set(b.keys())
        differences = []
        for key in sorted(all_keys):
            key_path = f"{path}.{key}" if path else key
            if key not in a:
                differences.append(key_path)
            elif key not in b:
                differences.append(key_path)
            else:
                sub_diff = json_diff(a[key], b[key], key_path)
                differences.extend(sub_diff)
        return sorted(differences)
    
    elif isinstance(a, list):
        if len(a) != len(b):
            return [path] if path else [""]
        differences = []
        for i in range(len(a)):
            item_path = f"{path}[{i}]"
            sub_diff = json_diff(a[i], b[i], item_path)
            differences.extend(sub_diff)
        return sorted(differences)
    
    else:
        return [path] if a != b else []