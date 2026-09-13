def clone(obj):
    memo = {}
    
    def _clone(o):
        if id(o) in memo:
            return memo[id(o)]
        
        if isinstance(o, dict):
            clone_dict = {}
            memo[id(o)] = clone_dict
            for k, v in o.items():
                clone_key = _clone(k)
                clone_value = _clone(v)
                clone_dict[clone_key] = clone_value
            return clone_dict
        elif isinstance(o, list):
            clone_list = []
            memo[id(o)] = clone_list
            for item in o:
                clone_list.append(_clone(item))
s_clone_list = clone_list
            return clone_list
        elif isinstance(o, tuple):
            return tuple(_clone(item) for item in o)
        else:
            return o
    
    return _clone(obj)