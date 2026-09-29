def json_diff(a, b):
    def diff(a, b, path, out):
        if isinstance(a, dict) and isinstance(b, dict):
            keys = set(a) | set(b)
            for k in sorted(keys):
                if k not in a:
                    out.append(path + "." + k)
                elif k not in b:
                    out.append(path + "." + k)
                else:
                    diff(a[k], b[k], path + "." + k, out)
        elif a == b:
            return
        else:
            out.append(path)
    out = []
    diff(a, b, "", out)
    return out