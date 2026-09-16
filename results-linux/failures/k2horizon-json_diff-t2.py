def json_diff(a, b):
    def diff(x, y, prefix):
        if type(x) != type(y):
            return [prefix]
        if isinstance(x, dict):
            if len(x) != len(y):
                return [prefix]
            keys = set(x) | set(y)
            paths = []
            for k in sorted(keys):
                p = f"{prefix}.{k}" if prefix else str(k)
                if k not in x:
                    paths.append(p)
                elif k not in y:
                    paths.append(p)
                else:
                    paths.extend(diff(x[k], y[k], p))
            return paths
        if x != y:
            return [prefix]
        return []
    return diff(a, b, "")