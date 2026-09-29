def clone(obj):
    # Map to track already cloned objects (original -> clone)
    memo = {}

    def _clone(o):
        # Base case: immutable types or None
        if o is None or isinstance(o, (int, float, str, bool, frozenset, type(...))):
            return o

        # Check if already cloned
        if o in memo:
            return memo[o]

        # Handle list
        if isinstance(o, list):
            new_obj = []
            memo[o] = new_obj
            for item in o:
                new_obj.append(_clone(item))
            return new_obj

        # Handle tuple
        if isinstance(o, tuple):
            new_obj = tuple(_clone(item) for item in o)
            return new_obj

        # Handle dict
        if isinstance(o, dict):
            new_obj = {}
            memo[o] = new_obj
            for k, v in o.items():
                new_obj[_clone(k)] = _clone(v)
            return new_obj

        # Fallback for other types (treat as immutable if not handled above)
        return o

    return _clone(obj)