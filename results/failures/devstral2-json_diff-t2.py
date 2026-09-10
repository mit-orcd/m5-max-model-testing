def json_diff(a, b):
    def diff_dict(da, db):
        keys = set(da.keys()) | set(db.keys())
        result = []
        for key in keys:
            path = (f"{key}" if not current_path else f"{current_path}.{key}")
            if key not in da:
                result.append((path, "missing", db[key]))
            elif key not in db:
                result.append((path, "extra", da[key]))
            elif isinstance(da[key], dict) and isinstance(db[key], dict):
                nested = diff_dict(da[key], db[key])
                for np in nested:
                    result.append((f"{path}.{np[0].split('.')[-1]}", np[1], np[2]))
            elif isinstance(da[key], list) and isinstance(db[key], list):
                if da[key] != db[key]:
                    result.append((path, "changed", (da[key], db[key])))
            else:
                if da[key] != db[key]:
                    result.append((path, "changed", (da[key], db[key])))
        return result

    def diff_list(la, lb):
        if la == lb:
            return []
        if len(la) != len(lb):
            return [("length", "changed", (len(la), len(lb)))]
        differences = []
        for i, (a, b) in enumerate(zip(la, lb)):
            if isinstance(a, dict) and isinstance(b, dict):
                nested = diff_dict(a, b)
                for np in nested:
                    differences.append((f"{i}.{np[0]}", np[1], np[2]))
            elif isinstance(a, list) and isinstance(b, list):
                if a != b:
                    differences.append((f"{i}", "changed", (a, b)))
            else:
                if a != b:
                    differences.append((f"{i}", "changed", (a, b)))
        return differences

    result = []

    if isinstance(a, dict) and isinstance(b, dict):
        nested = diff_dict(a, b)
        for path, status, value in nested:
            result.append((".", status, value))

    elif isinstance(a, list) and isinstance(b, list):
        nested = diff_list(a, b)
        for path, status, value in nested:
            result.append((".", status, value))

    else:
        if a != b:
            result.append((".", "changed", (a, b)))

    paths = []
    for path, status, value in result:
        if status == "missing":
            paths.append((path, "missing"))
        elif status == "extra":
            paths.append((path, "extra"))
        elif status == "changed":
            paths.append((path, "changed"))

    # Flatten the paths and sort
    flat_paths = []
    for path, status in paths:
        if "." in path:
            parts = path.split(".")
            for i in range(1, len(parts)):
                prefix = ".".join(parts[:i])
                flat_paths.append((prefix, status))
        flat_paths.append((path, status))

    unique_paths = list(set(flat_paths))
    unique_paths.sort()

    return [path for path, status in unique_paths]