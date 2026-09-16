def json_diff(a, b):
    def diff_path(path, a, b):
        if path not in diff:
            diff[path] = []
        if isinstance(a, dict) and isinstance(b, dict):
            for k in set(a.keys()) | set(b.keys()):
                diff_path(path + '.' + k, a.get(k), b.get(k))
        elif isinstance(a, list) and isinstance(b, list):
            if len(a) != len(b):
                diff[path].append('list lengths differ')
            else:
                for i in range(len(a)):
                    diff_path(path + '[' + str(i) + ']', a[i], b[i])
        else:
            if a != b:
                diff[path].append(repr(b))

    diff = {}
    diff_path('', a, b)
    return sorted(diff.keys())