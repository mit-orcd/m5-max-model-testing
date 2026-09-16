import json
from collections.abc import Mapping, Sequence

def json_diff(a, b):
    """
    Compares two JSON-like structures (dicts, lists, scalars).
    Returns a sorted list of dotted key paths where they differ.
    """
    def _compare(value_a, value_b, parent_path):
        # Handle types
        if isinstance(value_a, Mapping) and isinstance(value_b, Mapping):
            if value_a == value_b:
                return
            # Recurse into dicts
            keys_a = set(value_a.keys())
            keys_b = set(value_b.keys())
            
            # Keys only in a
            for key in keys_a - keys_b:
                result.append(parent_path + [str(key)])
            
            # Keys only in b
            for key in keys_b - keys_a:
                result.append(parent_path + [str(key)])
            
            # Common keys (recurse)
            for key in keys_a & keys_b:
                _compare(value_a[key], value_b[key], parent_path + [str(key)])
                
        elif isinstance(value_a, Sequence) and not isinstance(value_a, (str, bytes)):
            if value_a == value_b:
                return
            # Recurse into lists
            for i in range(max(len(value_a), len(value_b))):
                current_path = parent_path + [str(i)]
                # Handle index out of bounds
                val_a = value_a[i] if i < len(value_a) else None
                val_b = value_b[i] if i < len(value_b) else None
                _compare(val_a, val_b, current_path)
                
        else:
            # Scalars or different types
            if value_a != value_b:
                result.append(parent_path)

    result = []
    _compare(a, b, [])
    return sorted(result)