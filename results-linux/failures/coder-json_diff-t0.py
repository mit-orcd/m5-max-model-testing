def json_diff(a, b):
    def _diff(obj_a, obj_b, path=""):
        diffs = []
        if type(obj_a) != type(obj_b):
            diffs.append(path)
        elif isinstance(obj_a, dict):
            all_keys = set(obj_a.keys()) | set(obj_b.keys())
            for key in sorted(all_keys):
                new_path = f"{path}.{key}" if path else key
                if key not in obj_a:
                    diffs.append(new_path)
                elif key not in obj_b:
                    diffs.append(new_path)
                else:
                    diffs.extend(_diff(obj_a[key], obj_b[key], new_path))
        elif isinstance(obj_a, list):
            if len(obj_a) != len(obj_b):
                diffs.append(path)
            else:
                for i, (item_a, item_b) in enumerate(zip(obj_a, obj_b)):
                    new_path = f"{path}[{i}]"
                    diffs.extend(_diff(item_a, item_b, new_path))
        else:
            if obj_a != obj_b:
                diffs.append(path)
        return diffs
    
    return sorted(_diff(a, b))