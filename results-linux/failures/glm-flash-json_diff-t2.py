import json
from collections.abc import Mapping, Sequence

def json_diff(a, b):
    """
    Compares two JSON-like structures (dicts, lists, scalars).
    Returns a sorted list of dotted key paths where they differ.
    """
    def _compare(current_a, current_b, path):
        differences = []
        
        # Handle different types
        if type(current_a) != type(current_b):
            differences.append(path)
            return differences

        # Handle Dicts
        if isinstance(current_a, Mapping):
            # Keys present in only one side
            for key in set(current_a.keys()) | set(current_b.keys()):
                new_path = f"{path}.{key}" if path else key
                if key not in current_b:
                    differences.append(new_path)
                elif key not in current_a:
                    differences.append(new_path)
                else:
                    # Recurse into common keys
                    differences.extend(_compare(current_a[key], current_b[key], new_path))
            return differences

        # Handle Lists
        if isinstance(current_a, Sequence) and not isinstance(current_a, (str, bytes)):
            # Lists are compared by index
            length = max(len(current_a), len(current_b))
            for index in range(length):
                new_path = f"{path}[{index}]"
                a_val = current_a[index] if index < len(current_a) else None
                b_val = current_b[index] if index < len(current_b) else None
                differences.extend(_compare(a_val, b_val, new_path))
            return differences

        # Handle Scalars (including None, bool, int, float, str)
        if current_a != current_b:
            differences.append(path)
            return differences

        return differences

    return sorted(_compare(a, b, ""))