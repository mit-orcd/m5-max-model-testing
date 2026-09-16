def clone(obj, _seen=None):
    if _seen is None:
        _seen = {}
    obj_id = id(obj)
    if obj_id in _seen:
        return _seen[obj_id]
    if isinstance(obj, (int, float, str, bool, type(None))):
        return obj
    if isinstance(obj, tuple):
        new_obj = tuple(clone(item, _seen) for item in obj)
        _seen[obj_id] = new_obj
        return new_obj
    if isinstance(obj, list):
        new_obj = [clone(item, _seen) for item in obj]
        _seen[obj_id] = new_obj
        return new_obj
    if isinstance(obj, dict):
        new_obj = {clone(k, _seen): clone(v, _seen) for k, v in obj.items()}
        _seen[obj_id] = new_obj
        return new_obj
    raise TypeError(f"Unsupported type: {type(obj)}")