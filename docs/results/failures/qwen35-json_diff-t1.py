import json

def json_diff(a, b, path="root"):
    diff = []
    
    # Handle scalar comparison directly
    if not isinstance(a, (dict, list)) or not isinstance(b, (dict, list)):
        if a != b:
            diff.append(path)
        return diff
    
    # Both are dicts
    if isinstance(a, dict) and isinstance(b, dict):
        keys_a = set(a.keys())
        keys_b = set(b.keys())
        
        # Keys only in a
        for k in keys_a - keys_b:
            diff.append(f"{path}.{k}")
        
        # Keys only in b
        for k in keys_b - keys_a:
            diff.append(f"{path}.{k}")
        
        # Keys in both, recurse
        for k in keys_a & keys_b:
            diff.extend(json_diff(a[k], b[k], f"{path}.{k}"))
            
    # Both are lists
    elif isinstance(a, list) and isinstance(b, list):
        # Compare lists as whole values
        if a != b:
            diff.append(path)
            
    else:
        # One is dict/list and other is scalar (or mismatched types)
        diff.append(path)
        
    return sorted(diff)