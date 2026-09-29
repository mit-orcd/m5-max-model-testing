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
        elif isinstance(x, list):
            if len(x) != len(y):
                return [path] if path else []
            result = []
            for i, (item_x, item_y) in enumerate(zip(x, y)):
                new_path = f"{path}[{i}]" if path else f"[{i}]"
                result.extend(_diff(new_path, item_x, item_y))
            return result
        else:
            return [path] if x != y and path else []
    
    return sorted(_diff('', a, b))