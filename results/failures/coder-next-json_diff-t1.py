def json_diff(a, b):
    def _diff(path, x, y):
        # If both are dicts, recurse into keys
        if isinstance(x, dict) and isinstance(y, dict):
            result = []
            all_keys = set(x.keys()) | set(y.keys())
            for key in all_keys:
                if key in x and key in y:
                    result.extend(_diff(f"{path}.{key}" if path else key, x[key], y[key]))
                elif key in x:
                    result.append(f"{path}.{key}" if path else key)
                else:
                    result.append(f"{path}.{key}" if path else key)
            return result
        
        # If both are lists, compare by index
        if isinstance(x, list) and isinstance(y, list):
            result = []
            max_len = max(len(x), len(y))
            for i in range(max_len):
                idx_path = f"{path}.{i}" if path else str(i)
                if i < len(x) and i < len(y):
                    result.extend(_diff(idx_path, x[i], y[i]))
                elif i < len(x):
                    result.append(idx_path)
                else:
                    result.append(idx_path)
            return result
        
        # For scalars or mismatched types, compare directly
        if x != y:
            return [path] if path else []
        return []
    
    result = _diff("", a, b)
    return sorted(result)