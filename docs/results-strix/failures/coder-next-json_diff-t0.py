def json_diff(a, b):
    def _diff(path, x, y):
        if type(x) != type(y):
            return [path] if path else []
        
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
        
        if isinstance(x, list):
            if len(x) != len(y):
                return [path] if path else []
            result = []
            for i, (item_x, item_y) in enumerate(x):
                new_path = f"{path}[{i}]" if path else f"[{i}]"
                result.extend(_diff(new_path, item_x, item_y))
            return result
        
        # Scalar comparison
        if x != y:
            return [path] if path else []
        return []
    
    result = _diff("", a, b)
    return sorted(result)