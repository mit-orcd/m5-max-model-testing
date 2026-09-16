def json_diff(a, b):
    diffs = []

    def compare(x, y, path):
        if type(x) is not type(y):
            diffs.append(path)
            return
        if isinstance(x, dict):
            keys = set(x) | set(y)
            for k in sorted(keys):
                if k not in x or k not in y:
                    diffs.append(path + "." + k)
                else:
                    compare(x[k], y[k], path + "." + k)
        elif isinstance(x, list):
            if x != y:
                diffs.append(path)
        else:
            if x != y:
                diffs.append(path)

    compare(a, b, "")
    return sorted(diffs)