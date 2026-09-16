def json_diff(a, b):
    def _diff(path, x, y):
        # If types differ, they're different at this path
        if type(x) != type(y):
            return [path] if path else []
        
        # Both are dicts: recurse into keys
        if isinstance(x, dict):
            keys = set(x.keys()) | set(y.keys())
            result = []
            for key in keys:
                new_path = f"{path}.{key}" if path else key
                if key not in x:
                    result.append(new_path)
                elif key not in y:
                    result.append(new_path)
                else:
                    result.extend(_diff(new_path, x[key], y[key]))
            return result
        
        # Both are lists: compare element-by-element up to max length
        if isinstance(x, list):
            result = []
            max_len = max(len(x), len(y))
            for i in range(max_len):
                new_path = f"{path}.{i}" if path else str(i)
                if i >= len(x):
                    result.append(new_path)
                elif i >= len(y):
                    result.append(new_path)
                else:
                    result.extend(_diff(new_path, x[i], y[i]))
            return result
        
        # Scalars: compare directly
        return [path] if path and x != y else []
    
    result = _diff("", a, b)
    return sorted(result)