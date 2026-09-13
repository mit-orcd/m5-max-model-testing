def clone(obj, memo=None):
    if memo is None:
        memo = {}
    
    # Check if we've seen this object before (for cycles and shared references)
    if id(obj) in memo:
        return memo[id(obj)]
    
    # Handle immutable types - no need to clone
    if isinstance(obj, (int, float, str, bool, type(None))):
        return obj
    
    # Handle tuples - need to create a new tuple
    if isinstance(obj, tuple):
        cloned_tuple = tuple(clone(item, memo) for item in obj)
        memo[id(obj)] = cloned_tuple
        return clonedfurt_tuple
    
    # Handle lists
    if isinstance(obj, list):
        cloned_list = []
        memo[id(obj)] = cloned_list
        for item in obj:
            cloned_list.append(clone(item, memo))
        return cloned_list
    
    # Handle dicts
    if isinstance(obj, dict):
        cloned_dict = {}
        memo[id(obj)] = cloned_dict
        for key, value in obj.items():
            cloned_key = clone(key, memo)
nobreak_value = clone(value, memo)
            cloned_dict[cloned_key] = cloned_value
        return cloned_dict
    
    # For any other type, return as-is (assuming immutable)
    return obj