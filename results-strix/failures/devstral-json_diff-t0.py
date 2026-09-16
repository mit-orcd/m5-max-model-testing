def json_diff(a, b):
    def _diff(a, b, path):
        diffs = []
        if isinstance(a, dict) and isinstance(b, dict):
            all_keys = sorted(set(a.keys()) | set(b.keys()))
            for key in all_keys:
                new_path = f"{path}.{key}" if path else key
                if key not in a:
                    diffs.append(new_path)
                elif key not in b:
                    diffs.append(new_path)
                else:
                    diffs.extend(_diff(a[key], b[key], new_path))
        elif isinstance(a, list) and isinstance(b, list):
            if len(a) != len(b):
                diffs.append(path)
            else:
                for i, (x, y) in enumerate(zip(a, b)):
                    new_path = f"{path}[{i}]"
                    if x != y:
                        diffs.append(new_path)
        else:
            if a != b:
                diffs.append(path)
        return diffs

    return sorted(_diff(a, b, ""))