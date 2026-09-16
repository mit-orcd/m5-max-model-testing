def clone(obj, memo=None):
    if memo is None:
        memo = {}
    if id(obj) in memo:
        return memo[id(obj)]
    if isinstance(obj, dict):
        new_obj = {}
        memo[id(obj)] = new_obj
        for key, value in obj.items():
            new_obj[clone(key, memo)] = clone(value, memo)
        return new_obj
    elif isinstance(obj, (list, tuple)):
        new_obj = [] if isinstance(obj, list) else ()
        memo[id(obj)] = new_obj
        for item in obj:
            new_obj += (clone(item, memo),)
        return type(obj)(new_obj)
    else:
        return obj