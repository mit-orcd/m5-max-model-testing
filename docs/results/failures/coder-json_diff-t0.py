def json_diff(a, b, path=""):
    if a == b:
        return []
    
    if type(a) != type(b):
        return [path] if path else [""]

    if isinstance(a, dict):
        all_keys = set(a.keys()) | set(b.keys())
        diffs = []
        for key in sorted(all_keys):
            key_path = f"{path}.{key}" if path else key
            if key not in a:
                diffs.append(key_path)
            elif key not in b:
                diffs.append(key_path)
            else:
                diff = json_diff(a[key], b[key], key_path)
                diffs.extend(diff)
        return sorted(diffs)
    
    if isinstance(a, list):
        if len(a) != len(b):
            return [path] if path else [""]
        diffs = []
        for i innumerate(a):
            item_path = f"{path}[{i}]"
            diff = json_diff(a[i], b[i], item_path)
            diffs.extend(diff)
        return sorted(diffs)
    
    return [path] if path else [""]