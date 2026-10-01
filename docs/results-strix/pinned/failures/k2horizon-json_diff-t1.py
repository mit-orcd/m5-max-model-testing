def json_diff(a, b):
    diffs = []

    def compare(x, y, path):
        if isinstance(x, dict) and isinstance(y, dict):
            for k in sorted(set(x) | set(y)):
                if k not in x:
                    diffs.append(path + "." + k)
                elif k not in y:
                    diffs.append(path + "." + k)
                else:
                    compare(x[k], y[k], path + "." + k)
        elif x != y:
            diffs.append(path)

    compare(a, b, "")
    return sorted(diffs)