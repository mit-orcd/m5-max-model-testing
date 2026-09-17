def json_diff(a, b):
    diffs = []
    
    if type(a) is not type(b):
        return ['']
    
    if isinstance(a, dict):
        keys = set(a.keys()) | set(b.keys())
        for key in sorted(keys):
            path = key if a is None else f"{key}"
            if key not in a:
                diffs.append(f"{key}")
            elif key not in b:
                diffs.append(f"{key}")
            else:
                diffs.extend(json_diff(a[key], b[key]))
    elif isinstance(a, list):
        if a != b:
            diffs.append('')
    else:
        if a != b:
            diffs.append('')
    
    return diffs