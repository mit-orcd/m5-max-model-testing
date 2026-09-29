def json_diff(a, b, prefix=""):
    """
    Compares two JSON-like structures (dicts, lists, scalars).
    Returns a sorted list of dotted key paths where they differ.
    """
    diffs = []
    
    # Case 1: Both are dicts
    if isinstance(a, dict) and isinstance(b, dict):
        all_keys = set(a.keys()) | set(b.keys())
        for key in all_keys:
            path = f"{prefix}.{key}" if prefix else key
            if key not in a:
                diffs.append(f"{path} (missing in left)")
            elif key not in b:
                diffs.append(f"{path} (missing in right)")
            else:
                diffs.extend(json_diff(a[key], b[key], path))
    
    # Case 2: Both are lists (compared as whole values)
    elif isinstance(a, list) and isinstance(b, list):
        if a != b:
            diffs.append(prefix if prefix else "root")
            
    # Case 3: Both are scalars (or types match but values differ)
    elif type(a) == type(b):
        if a != b:
            diffs.append(prefix if prefix else "root")
            
    # Case 4: Type mismatch
    else:
        diffs.append(prefix if prefix else "root")
        
    return sorted(diffs)