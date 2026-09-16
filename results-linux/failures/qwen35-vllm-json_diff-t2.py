def json_diff(a, b, path=""):
    results = []
    
    if type(a) != type(b):
        results.append(path if path else "<root>")
        return results
    
    if isinstance(a, dict):
        keys = set(a.keys()) | set(b.keys())
        for key in sorted(keys):
            new_path = f"{path}.{key}" if path else key
            results.extend(json_diff(a.get(key), b.get(key), new_path))
    elif isinstance(a, list):
        len_a, len_b = len(a), len(b)
        if len_a != len_b:
            results.append(path if path else "<root>")
        else:
            for i in range(len_a):
                new_path = f"{path}[{i}]"
                results.extend(json_diff(a[i], b[i], new_path))
    else:
        if a != b:
            results.append(path if path else "<root>")
            
    return sorted(results)