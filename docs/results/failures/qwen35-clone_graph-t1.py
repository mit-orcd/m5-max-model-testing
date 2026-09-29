def clone(obj):
    memo = {}

    def _clone(item):
        # Handle immutable scalars directly
        if item is None or isinstance(item, (bool, int, float, complex, str, type(...), bytes)):
            return item
        
        # Handle tuples: create new tuple recursively
        if isinstance(item, tuple):
            return tuple(_clone(x) for x in item)

        # Handle mutable types that need deep copy logic
        if item in memo:
            return memo[item]

        if isinstance(item, dict):
            # Mark as being copied to handle cycles
            memo[item] = {}
            new_dict = {}
            for k, v in item.items():
                new_dict[k] = _clone(v)
            memo[item] = new_dict
            return new_dict

        if isinstance(item, list):
            # Mark as being copied to handle cycles
            memo[item] = []
            new_list = []
            for val in item:
                new_list.append(_clone(val))
            memo[item] = new_list
            return new_list

        if isinstance(item, set):
            # Sets are mutable
            memo[item] = set()
            new_set = set()
            for val in item:
                new_set.add(_clone(val))
            memo[item] = new_set
            return new_set

        if isinstance(item, frozenset):
            # Immutable sets
            return frozenset(_clone(x) for x in item)

        # For other unknown objects, return as-is (assuming they are immutable or handled by caller)
        # If the object supports identity comparison for cycle detection, we rely on 'item in memo'.
        # If it's a custom mutable object not listed above, standard behavior is to return it or raise.
        # Given the prompt specifies dicts, lists, tuples, and scalars, we assume no other mutables.
        if hasattr(item, '__dict__'):
             # If it's a custom object, we could try to copy attributes, but the prompt constraints 
             # focus on data structures. Returning as-is to avoid error, or raising NotImplementedError.
             # To strictly follow "Deep-copies a structure of dicts, lists, tuples and scalars", 
             # we assume inputs are limited to these or immutable.
             return item

        return item

    return _clone(obj)