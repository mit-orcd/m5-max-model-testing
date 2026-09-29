def json_diff(a, b):
    diffs = []
    
    def compare(x, y, path):
        if type(x) != type(y):
            diffs.append(path)
            return
        if isinstance(x, dict):
            for k in x:
                if k not in y:
                    diffs.append(path + "." + k)
                else:
                    compare(x[k], y[k], path + "." + k)
            for k in y:
                if k not in x:
                    diffs.append(path + "." + k)
        elif isinstance(x, list):
            if x != y:
                diffs.append(path)
        else:
            if x != y:
                diffs.append(path)
    
    compare(a, b, "")
    return sorted(diffs)