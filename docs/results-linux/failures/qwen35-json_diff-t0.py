def json_diff(a, b, prefix=""):
    result = []
    
    if type(a) != type(b):
        result.append(prefix if prefix else "<root>")
        return result
    
    if isinstance(a, dict):
        all_keys = set(a.keys()) | set(b.keys())
        for key in sorted(all_keys):
            new_prefix = f"{prefix}.{key}" if prefix else key
            if key not in a:
                result.append(new_prefix)
            elif key not in b:
                result.append(new_prefix)
            else:
                result.extend(json_diff(a[key], b[key], new_prefix))
    elif isinstance(a, list):
        max_len = max(len(a), len(b))
        for i in range(max_len):
            new_prefix = f"{prefix}[{i}]" if prefix else f"[{i}]"
            if i >= len(a):
                result.append(new_prefix)
            elif i >= len(b):
                result.append(new_prefix)
            elif a[i] != b[i]:
                # Recurse only if both are dicts, otherwise treat as scalar change
                if isinstance(a[i], dict) and isinstance(b[i], dict):
                    result.extend(json_diff(a[i], b[i], new_prefix))
                else:
                    result.append(new_prefix)
    else:
        if a != b:
            result.append(prefix if prefix else "<root>")
            
    return sorted(result)