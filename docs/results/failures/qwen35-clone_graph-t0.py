def clone(obj):
    import sys
    # Use a dictionary to track visited objects to handle cycles and shared references
    # Key: original object, Value: cloned object
    memo = {}

    def _clone(item):
        # Handle immutable scalars (str, int, float, bool, None, etc.)
        if not isinstance(item, (dict, list, tuple, set, frozenset)):
            return item

        # Check if already cloned (handles cycles and shared references)
        if item in memo:
            return memo[item]

        # Create a new container for the current item
        if isinstance(item, dict):
            new_item = {}
        elif isinstance(item, list):
            new_item = []
        elif isinstance(item, tuple):
            new_item = ()
        elif isinstance(item, set):
            new_item = set()
        elif isinstance(item, frozenset):
            new_item = frozenset()
        else:
            # Fallback for other types if any, though problem statement limits to dicts/lists/tuples/scalars
            return item

        # Register the new container in memo before processing contents to handle cycles
        memo[item] = new_item

        # Recursively clone contents
        if isinstance(item, dict):
            for k, v in item.items():
                new_item[k] = _clone(v)
        elif isinstance(item, list):
            for v in item:
                new_item.append(_clone(v))
        elif isinstance(item, tuple):
            for v in item:
                new_item += (_clone(v),)
        elif isinstance(item, set):
            for v in item:
                new_item.add(_clone(v))
        elif isinstance(item, frozenset):
            for v in item:
                new_item = new_item | frozenset([_clone(v)])

        return new_item

    return _clone(obj)