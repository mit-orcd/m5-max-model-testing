def clone(obj):
    memo = {}

    def _clone(o):
        if id(o) in memo:
            return memo[id(o)]
        if isinstance(o, (int, float, str, bool, type(None))):
            return o
        if isinstance(o, dict):
            new_dict = {}
            memo[id(o)] = new_dict
            for k, v in o.items():
                new_dict[_clone(k)] = _clone(v)
            return new_dict
        if isinstance(o, (list, tuple)):
            new_list = []
            memo[id(o)] = new_list
            for item in o:
                new_list.append(_clone(item))
            return type(o)(new_list)
        return o

    return _clone(obj)