def json_diff(a, b):
    def diff(a, b, path=''):
        if type(a) is not type(b):
            return ['{}: types differ'.format(path)]
        if isinstance(a, dict):
            diff_paths = []
            for k in set(a.keys()) | set(b.keys()):
                diff_paths.extend(diff(a.get(k), b.get(k), path + '.' + k))
            return diff_paths
        elif isinstance(a, list):
            if len(a) != len(b):
                return ['{}: list lengths differ'.format(path)]
            return [path + '.' + str(i) for i in range(len(a)) if a[i] != b[i]]
        else:
            return ['{}: values differ'.format(path)] if a != b else []

    return diff(a, b)