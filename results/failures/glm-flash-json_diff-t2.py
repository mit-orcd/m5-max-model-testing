import json
from collections.abc import Mapping, Sequence

def json_diff(a, b, parent_path=''):
    """
    Recursively compares two JSON-like structures (dicts, lists, scalars).
    Returns a sorted list of dotted key paths where they differ.
    """
    differences = []

    if isinstance(a, Mapping) and isinstance(b, Mapping):
        # Compare Dicts
        all_keys = sorted(set(a.keys()).union(set(b.keys())))
        
        for key in all_keys:
            path = f"{parent_path}.{key}" if parent_path else key
            if key not in a:
                differences.append(f"{path} [only in b]")
                continue
            if key not in b:
                differences.append(f"{path} [only in a]")
                continue
            
            sub_diff = json_diff(a[key], b[key], path)
            differences.extend(sub_diff)

    elif isinstance(a, Sequence) and not isinstance(a, str) and isinstance(b, Sequence) and not isinstance(b, str):
        # Compare Lists
        if len(a) != len(b):
            differences.append(parent_path if parent_path else "[list length differs]")
            return differences
        
        for index0, (item_a, item_b) in enumerate(zip(a, b)):
            index_path = f"{parent_path}[{index0}]"
            if isinstance(item_a, Mapping) or isinstance(item2, Mapping):
                 # Handle nested non-scalar objects in lists (using dummy key)
                 diff = json_diff(item_a, item_b, index_path)
            else:
                 # Compare scalars
                 if item_a != item_b:
                     differences.append(index_path)
            differences.extend(diff if isinstance(diff, list) else [diff])

    # Compare Scalars
    elif a != b:
        differences.append(parent_path if parent_path else str(a))

    return sorted(differences)