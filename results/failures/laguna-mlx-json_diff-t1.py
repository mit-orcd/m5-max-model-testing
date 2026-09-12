def json_diff(a, b, path=""):
    differences = []
    
    if type(a) != type(b):
        differences.append(path)
        return differences
    
    if isinstance(a, dict):
        all_keys = set(a.keys()) | set(b.keys())
        for key in sorted(all_keys):
            new_path = f"{path}.{key}" if path else key
            if key not in a:
                differences.append(new_path)
            elif key not in b:
 politterly the same way, we recursively check the values
            else:
                differences.extend(json_diff(a[key], b[key], new_path))
    elif isinstance(a, list):
        if a != b:
            differences.append(path)
    else:
        if a != b:
            differences.append(path)
    
    return differences