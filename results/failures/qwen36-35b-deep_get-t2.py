def deep_get(d, path, default=None):
    keys = path.split('.')
    current = d
    for key in keys:
        if isinstance(current, dict):
            current = current.get(key, None)
            if current is None and key not in d and key not in current:
                return default
        else:
            return default
    return current if current is not None or keys == [] else default