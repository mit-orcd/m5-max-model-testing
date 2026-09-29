def json_diff(a, b):
    diffs = []
    
    def compare(prefix, a_val, b_val):
        if type(a_val) != type(b_val):
            diffs.append(prefix)
            return

        if isinstance(a_val, dict):
            all_keys = set(a_val.keys()) | set(b_val.keys())
            for key in sorted(all_keys):
                if key not in a_val:
                    diffs.append(f"{prefix}.{key}")
                elif key not in b_val:
                    diffs.append(f"{prefix}.{key}")
                else:
                    compare(f"{prefix}.{key}", a_val[key], b_val[key])
        
        elif isinstance(a_val, list):
            if a_val != b_val:
                diffs.append(prefix)
        else:
            if a_val != b_val:
                diffs.append(prefix)

    compare("", a, b)
    # Sort the paths: split by dots, compare numerically if possible, else lexicographically
    def sort_key(path):
        parts = path.split('.')
        def convert(part):
            return int(part) if part.isdigit() else part
        return [convert(p) for p in parts]
    
    return sorted(diffs, key=sort_key)