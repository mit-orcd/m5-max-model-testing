def json_diff(a, b):
    def diff_path(path, a, b):
        if isinstance(a, dict) and isinstance(b, dict):
            return [
                f"{path}.{k}"
                for k in set(a.keys()) ^ set(b.keys())
                for v in diff_path([path, k], a[k], b[k])
            ]
        elif isinstance(a, list) and isinstance(b, list):
            return [
                f"{path}[{'+'.join(map(str, set(b) - set(a)))}]"
                if len(set(a) ^ set(b))
                else f"{path}[{'+'.join(map(str, set(a) - set(b)))}]"
            ]
        else:
            return [f"{path}" if a != b else []]

    return sorted(diff_path("", a, b))