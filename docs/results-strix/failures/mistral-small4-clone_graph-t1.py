def clone(obj):
    if isinstance(obj, dict):
        new_obj = {}
        for k, v in obj.items():
            new_obj[clone(k)] = clone(v)
        return new_obj
    elif isinstance(obj, list):
        new_obj = []
        for item in obj:
            new_obj.append(clone(item))
        return new_obj
    elif isinstance(obj, tuple):
        return tuple(clone(item) for item in obj)
    else:
        return obj