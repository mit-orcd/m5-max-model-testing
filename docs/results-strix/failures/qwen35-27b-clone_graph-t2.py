def clone(obj):
    memo = {}

    def _clone(x):
        if x in memo:
            return memo[x]
        
        # Handle immutable scalars (int, float, str, bool, None, tuple of immutables)
        # Note: tuples are handled specially below to support cycles with mutable elements
        if isinstance(x, (int, float, str, bool, type(None))):
            return x
        
        if isinstance(x, tuple):
            memo[x] = tuple(_clone(item) for item in x)
            return memo[x]
        
        if isinstance(x, list):
            memo[x] = []
            result = memo[x]
            for item in x:
                result.append(_clone(item))
            return result
        
        if isinstance(x, dict):
            memo[x] = {}
            result = memo[x]
            for k, v in x.items():
                # Keys must be immutable for dicts, so we don't clone them for identity
                # but we do clone values.
                result[_clone(k)] = _clone(v)
            return result
        
        # For any other type (including custom objects or sets), return as is or raise
        # Based on problem statement, we only care about dicts, lists, tuples, scalars.
        # If a scalar-like type is passed that isn't in the tuple above, return it.
        return x

    return _clone(obj)