def json_diff(a, b):
    def diff(a, b, path=None):
        if path is None:
            path = []
        if isinstance(a, dict) and isinstance(b, dict):
            return [
                [path + [k]] + diff(a[k], b.get(k), path + [k])
                for k in set(a) ^ set(b)
                + set(b) - set(a)
            ] + [
                [path + [k]] + diff(a[k], b[k], path + [k])
                for k in set(a) & set(b)
            ]
        elif isinstance(a, list) and isinstance(b, list):
            return [
                path + [i] + diff(a[i], b[i], path + [i])
                for i in range(len(a))
                if a[i] != b[i]
            ]
        else:
            return [path] * (a != b)
        return []

    return diff(a, b)