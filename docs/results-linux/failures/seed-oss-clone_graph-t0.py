def clone(obj, memo=None):
    if memo is None:
        memo = {}
    if obj in memo:
        return memo[obj]
    if isinstance(obj, list):
        cloned = []
        memo[obj] = cloned
        for item in obj:
            cloned.append(clone(item, memo))
        return cloned
    elif isinstance(obj, dict):
        cloned = {}
        memo[obj] = cloned
        for k, v in obj.items():
            cloned[clone(k, memo)] = clone(v, memo)
        return cloned
    elif isinstance(obj, tuple):
        cloned_elements = []
        memo[obj] = cloned_elements
        for item in obj:
            cloned_elements.append(clone(item, memo))
        cloned_tuple = tuple(cloned_elements)
        memo[obj] = cloned_tuple
        return cloned_tuple
    else:
        return obj