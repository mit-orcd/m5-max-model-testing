def json_diff(a, b):
    out = []

    def rec(x, y, path):
        if isinstance(x, dict) and isinstance(y, dict):
            for k in sorted(set(x) | set(y)):
                p = f"{path}.{k}" if path else k
                if k not in x or k not in y:
                    out.append(p)
                else:
                    rec(x[k], y[k], p)
        elif x != y:
            out.append(path)

    rec(a, b, "")
    return sorted(out)
