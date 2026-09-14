def clone(obj):
    memo = {}

    def _clone(x):
        # Handle scalars (int, float, str, bool, None, etc.)
        if isinstance(x, (int, float, str, bool, type(None))):
            return x
        
        # Handle immutable collections (tuple, frozenset)
        if isinstance(x, tuple):
            if x in memo:
                return memo[x]
            new_tuple = tuple(_clone(item) for item in x)
            memo[x] = new_tuple
            return new_tuple
        
        if isinstance(x, frozenset):
            if x in memo:
                return memo[x]
            new_frozenset = frozenset(_clone(item) for item in x)
            memo[x] = new_frozenset
            return new_frozenset
        
        # Handle mutable collections that can create cycles
        if isinstance(x, dict):
            if id(x) in memo:
                return memo[id(x)]
            new_dict = {}
            memo[id(x)] = new_dict  # Create a placeholder to handle cycles
            for k, v in x.items():
                new_dict[_clone(k)] = _clone(v)
            return new_dict
        
        if isinstance(x, list):
            if id(x) in memo:
                return memo[id(x)]
            new_list = []
            memo[id(x)] = new_list  # Create a placeholder to handle cycles
            for item in x:
                new_list.append(_clone(item))
            return new_list
        
        # Default behavior for other objects (treat as scalars to avoid errors)
        return x

    return _clone(obj)