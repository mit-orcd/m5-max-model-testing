def clone(obj, memo=None):
    if memo is None:
        memo = {}
    obj_id = id(obj)
    if obj_id in memo:
        return memo[obj_id]
    if isinstance(obj, dict):
        new_obj = {}
        memo[obj_id] = new_obj
        for key, value in obj.items():
            new_obj[clone(key, memo)] = clone(value, memo)
        return new_obj
    elif isinstance(obj, (list, tuple)):
        new_obj = [] if isinstance(obj, list) else ()
        memo[obj_id] = new_obj
        for item in obj:
            new_obj.append(clone(item, memo))
        return new_obj
    else:
        return obj