def json_diff(a, b):
    def _diff(path, x, y):
        # If types differ, they're different at this path
        if type(x) != type(y):
            return [path] if path else []
        
        # Handle dicts
        if isinstance(x, dict):
            result = []
            all_keys = set(x.keys()) | set(y.keys())
            for key in all_keys:
                new_path = f"{path}.{key}" if path else key
                if key not in x:
                    result.append(new_path)
                elif key not in y:
                    result.append(new_path)
                else:
                    result.extend(_diff(new_path, x[key], y[key]))
            return result
        
        # Handle lists
        if isinstance(x, list):
            result = []
            max_len = max(len(x), len(y))
            for i in range(max_len):
                new_path = f"{path}[{i}]" if path else f"[{i}]"
                if i >= len(x):
                    result.append(new_path)
                elif i >= len(y):
                    result.append(new_path)
                else:
                    result.extend(_diff(new_path, x[i], y[i]))
            return result
        
        # Handle scalars (int, float, str, bool, None)
        if x != y:
            return [path] if path else []
        
        return []
    
    return sorted(_diff("", a, b))