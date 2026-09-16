def json_diff(a, b):
    def _diff(a, b, path):
        if type(a) != type(b):
            diffs.append(path)
            return
        if isinstance(a, dict):
            all_keys = set(a.keys()) | set(b.keys())
            for key in sorted(all_keys):
                new_path = f"{path}.{key}" if path else key
                if key not in a or key not in b:
                    diffs.append(new_path)
                else:
                    _diff(a[key], b[key], new_path)
        elif isinstance(a, list):
            if len(a) != len(b):
                diffs.append(path)
            else:
                for i in range(len(a)):
                    _diff(a[i], b[i], f"{path}[{i}]")
        else:
            if a != b:
                diffs.append(path)

    diffs = []
    _diff(a, b, "")
    return sorted(diffs)