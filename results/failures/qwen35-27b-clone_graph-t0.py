def clone(obj):
    memo = {}

    def _clone(o):
        if o is None:
            return None
        if isinstance(o, (int, float, str, bool, type(...), type(NotImplemented))):
            return o
        if isinstance(o, tuple):
            if o in memo:
                return memo[o]
            memo[o] = tuple(_clone(item) for item in o)
            return memo[o]
        if isinstance(o, list):
            if o in memo:
                return memo[o]
            memo[o] = []
            for item in o:
                memo[o].append(_clone(item))
            return memo[o]
        if isinstance(o, dict):
            if o in memo:
                return memo[o]
            memo[o] = {}
            for k, v in o.items():
                memo[o][_clone(k)] = _clone(v)
            return memo[o]
        # Fallback for other types (should not occur per spec)
        return o

    return _clone(obj)