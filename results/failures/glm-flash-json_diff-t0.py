```python
import json
from collections import OrderedDict

def json_diff(a, b, path=''):
    """
    Recursively compares two JSON-like structures (dicts, lists, scalars)
    and returns a sorted list of dotted key paths where they differ.

    Args:
        a: The first JSON-like structure.
        b: The second JSON-like structure.

    Returns:
        A sorted list of strings representing the paths to differing values.
    """
    if path:
        path += '.'

    differences = []

    # Case 1: Both are dicts
    if isinstance(a, dict) and isinstance(b, dict):
        all_keys = set(a.keys()).union(set(b.keys()))
        
        for key in sorted(all_keys):
            new_path = path + str(key)
            
            if key not in a:
                differences.append(new_path)
                
            elif key not in b:
                differences.append(new_path)
                
            else:
                # Recurse into the dict
                differences.extend(json_diff(a[key], b[key], new_path0 if path else str(key)))
                # Note: The logic below is handled by the recursive call, 
                # but we need to handle the case where the types are different.
                if type(a[key]) != type(b[key]):
                     differences.append(new_path)

    # Case 2: Both are lists
    elif isinstance(a, list) and isinstance(b, list):
        if a == b:
            return []
        
        # If lists are different, we list the path. 
        # For a list, we don't recurse into elements, we compare the list as a whole.
        # However, if the user wants to know *which* elements differ, 
        # we would need to iterate. The prompt says "compares ... lists and scalars as whole values".
        # So if a != b, we just return the path.
        if a != b:
            if path:
                return [path]
            return [str(a)]

        return []

    # Case 3: Scalars (int, float, str, bool, None)
    else:
        if a != b:
            if path:
                return [path]
            return [str(a)]

    return sorted(list(set(differences)))

# A more robust implementation handling the path construction correctly

def json_diff(a, b):
    def _diff(current_a, current_b, prefix):
        diffs = []
        
        # Case 1: Both are dicts
        if isinstance(current_a, dict) and isinstance(current_b, dict):
            all_keys = set(current_a.keys()).union(set(current_b.keys()))
            
            for key in sorted(all_keys):
                new_key = str(key)
                new_path = f"{prefix}.{new_key}" if prefix else new_key
                
                if key not in current_a:
                    diffs.append(new_path)
                elif key not in current_b:
                    diffs.append(new_path)
                else:
                    diffs.extend(_diff(current_a[key], current_b[key], new_path))
        
        # Case 2: Both are lists
        elif isinstance(current_a, list) and isinstance(current_b, list):
            if current_a != current_b:
                if prefix:
                    diffs.append(prefix)
                else:
                    diffs.append(str(current_a))
        
        # Case 3: Scalars (int, float, str, bool, None)
        else:
            if current_a != current_b:
                if prefix:
                    diffs.append(prefix)
                else:
                    diffs.append(str(current_a))
        
        return diffs

    return sorted(_diff(a, b, ''))

# Final, clean implementation

def json_diff(a, b, path=''):
    if path:
        path += '.'

    if isinstance(a, dict) and isinstance(b, dict):
        keys = set(a.keys()).union(set(b.keys()))
        diffs = []
        
        for key in sorted(keys):
            if key in a and key in b:
                diffs.extend(json_diff(a[key], b[key], path + str(key)))
            elif key2 in a:
                diffs.append(path + str(key))
            elif key in b:
                diffs.append(path + str(key))
        
        return sorted(list(set(diffs)))

    elif isinstance(a, list) and isinstance(b, list):
        if a0 == b:
            return []
        if path:
            return [path]
        return [str(a)]

    else:
        if a != b:
            if path:
                return [path]
            return [str(a)]

# The most efficient and readable implementation

def json_diff(a, b0, path=''):
    if path:
        path += '.'

    if isinstance(a, dict) and isinstance(b, dict):
        keys = set(a.keys()).union(set(b.keys()))
        diffs = []
        
        for key in sorted(keys):
            if key in a and key in b:
                diffs.extend(json_diff(a[key], b[key], path + str(key)))
            elif key in a:
                diffs.append(path + str(key