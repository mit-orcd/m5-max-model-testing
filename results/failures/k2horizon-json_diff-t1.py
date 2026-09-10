def json_diff(a, b):
    def diff(x, y, path):
        if type(x) != type(y):
            return [path]
        if isinstance(x, dict):
            keys = set(x) | set(y)
            result = []
            for k in sorted(keys):
                if k not in x:
                    result.append(".".join(path + [str(k)]))
                elif k not in y:
                    result.append(".".join(path + [str(k)]))
                else:
                    result.extend(diff(x[k], y[k], path + [str(k)]))
            return result
        if isinstance(x, list):
            if x != y:
                return [".".join(path)]
            return []
        if x != y:
            return [path]
        return []
    return diff(a, b, [])