def json_diff(a, b):
    def _diff(x, y, path):
        if type(x) != type(y):
            return [path]
        if isinstance(x, dict):
            keys = set(x.keys()) | set(y.keys())
            diffs = []
            for k in sorted(keys):
                if k not in x:
                    diffs.append(_diff(None, y[k], path + [k]))
                elif k not in y:
                    diffs.append(_diff(x[k], None, path + [k]))
                else:
                    diffs.extend(_diff(x[k], y[k], path + [k]))
            return [item for sublist in diffs for item in sublist]
        elif isinstance(x, list):
            if x != y:
                return [path]
            return []
        else:
            if x != y:
                return [path]
            return []
    result = _diff(a, b, [])
    return ['.'.join(map(str, p)) for p in result]