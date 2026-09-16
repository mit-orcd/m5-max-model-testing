def clone(obj, _seen=None):
    if _seen is None:
        _seen = {}
    obj_id = id(obj)
    if obj_id in _seen:
        return _seen[obj_id]
    if isinstance(obj, (int, float, str, bool, type(None))):
        return obj
    if isinstance(obj, tuple):
        seen_tuple = _seen.get(obj_id)
        if seen_tuple is not None:
            return seen_tuple
        new_tuple = tuple(clone(item, _seen) for item in obj)
        _seen[obj_id] = new_tuple
        return new_tuple
    if isinstance(obj, list):
        seen_list = _seen.get(obj_id)
        if seen_list is not None:
            return seen_list
        new_list = [clone(item, _seen) for item in obj]
        _seen[obj_id] = new_list
        return new_list
    if isinstance(obj, dict):
        seen_dict = _seen.get(obj_id)
        if seen_dict is not None:
            return seen_dict
        new_dict = {clone(k, _seen): clone(v, _seen) for k, v in obj.items()}
        _seen[obj_id] = new_dict
        return new_dict
    raise TypeError(f"Unsupported type: {type(obj)}")