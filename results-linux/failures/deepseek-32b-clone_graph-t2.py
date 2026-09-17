def clone(obj, memo=None):
    if memo is None:
        memo = {}
    if obj in memo:
        return memo[obj]
    if isinstance(obj, dict):
        new_obj = {}
        memo[obj] = new_obj
        for key, value in obj.items():
            new_key = clone(key, memo)
            new_value = clone(value, memo)
            new_obj[new_key] = new_value
        return new_obj
    elif isinstance(obj, list):
        new_obj = []
        memo[obj] = new_obj
        for item in obj:
            new_item = clone(item, memo)
            new_obj.append(new_item)
        return new_obj
    elif isinstance(obj, tuple):
        new_list = []
        for item in obj:
            new_item = clone(item, memo)
            new_list.append(new_item)
        new_obj = tuple(new_list)
        memo[obj] = new_obj
        return new_obj
    else:
        return obj