def json_diff(a, b):
    def compare(path, a, b):
        if type(a) != type(b):
            return [path]
        if isinstance(a, dict):
            keys_a, keys_b = set(a.keys()), set(b.keys())
            if keys_a != keys_b:
                return [path] if not path else [path]
            diffs = []
            for key in sorted(keys_a & keys_b):
                child_path = f"{path}.{key}" if path else key
                diffs.extend(compare(child_path, a[key], b[key]))
            return diffs
        elif isinstance(a, list):
            if a != b:
                return [path] if path else [path]
            return []
        else:
            if a != b:
                return [path] if path else [path]
            return []
    
    return sorted(compare("", a, b))