def json_diff(a, b):
    def _diff(val_a, val_b, path):
        if isinstance(val_a, dict) and isinstance(val_b, dict):
            keys_a = set(val_a.keys())
            keys_b = set(val_b.keys())
            all_keys = keys_a | keys_b
            results = []
            for key in sorted(all_keys):
                new_path = f"{path}.{key}" if path else key
                if key not in keys_a:
                    results.append(new_path)
                elif key not in keys_b:
                    results.append(new_path)
                else:
                    results.extend(_diff(val_a[key], val_b[key], new_path))
            return results
        elif isinstance(val_a, list) and isinstance(val_b, list):
            if len(val_a) != len(val_b):
                return [path] if path else []
            results = []
            for i in range(len(val_a)):
                new_path = f"{path}[{i}]"
                results.extend(_diff(val_a[i], val_b[i], new_path))
            return results
        else:
            if val_a != val_b:
                return [path] if path else []
            return []
    
    return sorted(_diff(a, b, ""))