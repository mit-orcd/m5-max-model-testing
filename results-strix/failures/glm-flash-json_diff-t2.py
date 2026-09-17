from collections import OrderedDict

def json_diff(a, b):
    def recurse(current_a, current_b, path=""):
        diffs = []
        
        # Check if both are dicts
        if isinstance(current_a, dict) and isinstance(current_b, dict):
            # Keys in A only
            for key in current_a:
                if key not in current_b:
                    diffs.append(f"{path}.{key}")
            # Keys in B only
            for key in current_b:
                if key not in current_a:
                    diffs.append(f"{path}.{key}")
            # Keys in both
            for key in current_a:
                if key in current_b:
                    new_path = f"{path}.{key}" if path else key
                    diffs.extend(recurse(current_a[key], current_b[key], new_path))
                    
        # Check if both are lists
        elif isinstance(current_a, list) and isinstance(current_b, list):
            if current_a != current_b:
                diffs.append(path)
                
        # Check for scalar values (including mismatched types)
        else:
            if current_a != current_b:
                diffs.append(path)
                
        return diffs

    # Ensure deterministic ordering of results
    return sorted(recurse(a, b))