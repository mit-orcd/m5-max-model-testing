def json_diff(a, b):
    if type(a) != type(b):
        return ['']
    
    if isinstance(a, dict):
        keys = set(a.keys()) | set(b.keys())
        diffs = []
        for k in keys:
            if k in a and k not in b:
                diffs.append(f'.{k}')
            elif k not in a and k in b:
                diffs.append(f'.{k}')
            else:
                sub_diffs = json_diff(a[k], b[k])
                for path in sub_diffs:
                    diffs.append(f'.{k}{path}')
        return sorted(diffs)
    
    if isinstance(a, list):
        if a == b:
            return []
        else:
            return ['']
    
    if a == b:
        return []
    else:
        return ['']