def clone(obj):
    memo = {}
    def deep_copy(o):
        if o is obj:
            return obj
        if id(o) in memo:
            return memo[id(o)]
        if isinstance(o, dict):
            new_dict = {}
            memo[id(o)] = new_dict
            for k, v in o.items():
                new_dict[deep_copy(k)] = deep_copy(v)
            return new_dict
        elif isinstance(o, list):
            new_list = []
            memo[id(o)] = new_list
            for item in o:
                new_list.append(deep_copy(item))
            return new_list
        elif isinstance(o, tuple):
            new_tuple = tuple(deep_copy(item) for item in o)
            memo[id(o)] = new_tuple
            return new_tuple
        else:
            return o
    return deep_copy(obj)