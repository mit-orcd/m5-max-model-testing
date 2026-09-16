def json_diff(a, b):
    def diff_path(path, a, b):
        if isinstance(a, dict) and isinstance(b, dict):
            return [
                f"{path}.{k}"
                for k in set(a.keys()) ^ set(b.keys())
                for subdiff in diff_path(f"{path}.{k}", a[k], b.get(k, {}))
            ]
        elif isinstance(a, list) and isinstance(b, list):
            return [
                f"{path}"
                for i in range(max(len(a), len(b)))
                for subdiff in diff_path(f"{path}[{i}]", a[i] if i < len(a) else None, b[i])
            ]
        else:
            return [path] if a != b else []

    return diff_path("", a, b)