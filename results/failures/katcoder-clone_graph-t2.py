def clone(obj):
    memo = {}

    def _clone(o):
        typ = type(o)
        if typ in (int, float, str, bytes, type(None), bool):
            return o
        if typ is tuple:
            return tuple(_clone(x) for x in o)
        if o in memo:
            return memo[o]
        if isinstance(o, dict):
            clone = {}
            memo[o] = clone
            for k, v in o.items():
                clone[_clone(k)] = _clone(v)
            return clone
        if isinstance(o, list):
            clone = []
            memo[o] = clone
            for x in o:
                clone.append(_clone(x))
            return clone
        # For other mutable types, use shallow copy as fallback
        if hasattr(o, '__dict__'):
            clone = typ.__new__(typ)
            memo[o] = clone
            if hasattr(o, '__dict__'):
                clone.__dict__ = {k: _clone(v) for k, v in o.__dict__.items()}
            return clone
        return o

    return _clone(obj)