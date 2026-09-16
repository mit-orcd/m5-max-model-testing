def json_diff(a, b, prefix=""):
    results = []
    
    if type(a) is type(b) and isinstance(a, dict):
        all_keys = sorted(set(a.keys()) | set(b.keys()))
        for key in all_keys:
            new_prefix = f"{prefix}.{key}" if prefix else key
            if key not in a:
                results.append(new_prefix)
            elif key not in b:
                results.append(new_prefix)
            else:
                results.extend(json_diff(a[key], b[key], new_prefix))
    elif isinstance(a, list) and isinstance(b, list):
        max_len = max(len(a), len(b))
        for i in range(max_len):
            new_prefix = f"{prefix}[{i}]"
            if i >= len(a):
                results.append(new_prefix)
            elif i >= len(b):
                results.append(new_prefix)
            elif a[i] == b[i]:
                continue
            elif isinstance(a[i], dict) and isinstance(b[i], dict):
                results.extend(json_diff(a[i], b[i], new_prefix))
            else:
                results.append(new_prefix)
    else:
        if a != b:
            if prefix:
                results.append(prefix)
            else:
                results.append("")
    
    return results