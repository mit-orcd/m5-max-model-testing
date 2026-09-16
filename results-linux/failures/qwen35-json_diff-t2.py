def json_diff(a, b, path=""):
    result = []
    
    if type(a) != type(b):
        result.append(path if path else "<root>")
        return result
    
    if isinstance(a, dict):
        keys_a = set(a.keys())
        keys_b = set(b.keys())
        
        for key in keys_a - keys_b:
            result.append(f"{path}.{key}" if path else key)
        for key in keys_b - keys_a:
            result.append(f"{path}.{key}" if path else key)
            
        for key in keys_a & keys_b:
            new_path = f"{path}.{key}" if path else key
            result.extend(json_diff(a[key], b[key], new_path))
            
    elif isinstance(a, list):
        len_a, len_b = len(a), len(b)
        min_len = min(len_a, len_b)
        
        if len_a != len_b:
            result.append(path if path else "<root>")
        else:
            for i in range(min_len):
                new_path = f"{path}[{i}]"
                result.extend(json_diff(a[i], b[i], new_path))
                
    else:
        if a != b:
            result.append(path if path else "<root>")
            
    return sorted(result)