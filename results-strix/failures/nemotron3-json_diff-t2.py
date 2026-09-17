def json_diff(a, b):
    def _diff(a, b, path):
        if a == b:
            return []
        if isinstance(a, dict) and isinstance(b, dict):
            keys = set(a.keys()) | set(b.keys())
            diffs = []
            for k in sorted(keys):
                new_path = path + [k]
                if k not in a:
                    diffs.append('.'.join(new_path) + ' (missing in first)')
                elif k not in b:
                    diffs.append('.'.join(new_path) + ' (missing in second)')
                else:
                    diffs.extend(_diff(a[k], b[k], new_path))
            return diffs
        elif isinstance(a, list) and isinstance(b, list):
            if a != b:
                return ['.'.join(path)]
            return []
        else:
            return ['.'.join(path)]
    return _diff(a, b, [])