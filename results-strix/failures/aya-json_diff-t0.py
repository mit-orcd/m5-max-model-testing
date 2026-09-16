def json_diff(a, b):
    def diff_path(path, a, b):
        if isinstance(a, dict) and isinstance(b, dict):
            return [
                f"{path}.{k}"
                for k in set(a.keys()) ^ set(b.keys())
                for subpath in diff_path(f"{path}.{k}", a[k], b.get(k, {}))
            ]
        elif isinstance(a, list) and isinstance(b, list):
            return [
                f"{path}[{i}]"
                for i, (av, bv) in enumerate(zip(a, b))
                for subpath in diff_path(f"{path}[i]", av, bv)
            ]
        else:
            return [path] if a != b else []

    return diff_path("", a, b)