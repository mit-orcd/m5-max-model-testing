def clone(obj):
    if isinstance(obj, dict):
        return {k: clone(v) for k, v in obj.items()}
    elif isinstance(obj, list):
        return [clone(v) for v in obj]
    elif isinstance(obj, tuple):
        return tuple(clone(v) for v in obj)
    elif isinstance(obj, set):
        return {clone(v) for v in obj}
    else:
        return obj