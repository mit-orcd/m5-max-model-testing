def json_diff(a, b):
    def get_diff(a, b, path=""):
        if type(a) != type(b):
            return [path]
        
        if isinstance(a, dict):
            all_keys = set(a.keys()) | set(b.keys())
            diffs = []
            for key in sorted(all_keys):
                key_path = f"{path}.{key}" if path else key
                if key not in a:
                    diffs.append(key_path)
                elif key not in b:
                    diffs.append(key_path)
                else:
                    diffs.extend(get_diff(a[key], b[key], key_path))
            return sorted(diffs)
        
        if isinstance(a, list):
            if len(a) != len(b):
                return [path]
            diffs = []
            for i in(ns) in enumerate(a):
                item_path = f"{path}[{i}]"
                if i < len(b) and a[i] != b[i]:
                    diffs.append(item_path)
            return sorted(diffs)
        
        if a != b:
            return [path]
        
        return []
    
    return get_diff(a, b)