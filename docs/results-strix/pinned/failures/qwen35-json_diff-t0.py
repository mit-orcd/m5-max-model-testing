def json_diff(a, b, path=""):
    result = []
    
    if type(a) != type(b):
        result.append(path if path else "<root>")
        return result
    
    if isinstance(a, dict):
        all_keys = set(a.keys()) | set(b.keys())
        for key in sorted(all_keys):
            new_path = f"{path}.{key}" if path else key
            result.extend(json_diff(a.get(key), b.get(key), new_path))
    elif isinstance(a, list):
        max_len = max(len(a), len(b))
        for i in range(max_len):
            new_path = f"{path}[{i}]"
            val_a = a[i] if i < len(a) else None
            val_b = b[i] if i < len(b) else None
            if val_a != val_b:
                result.append(new_path)
    else:
        if a != b:
            result.append(path if path else "<root>")
            
    return result