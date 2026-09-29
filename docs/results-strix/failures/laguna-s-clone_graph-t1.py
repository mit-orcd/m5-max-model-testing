def clone(obj):
    memo = {}
    def _clone(o):
        if id(o) in memo:
            return memo[id(o)]
        if isinstance(o, dict):
            memo[id(o)] = {}
            return {k: _clone(v) for k, v in o.items()}
        if isinstance(o, list):
            memo[id(o)] = []
            return [_clone(v) for v in o]
        if isinstance(o, tuple):
            memo[id(o)] = tuple()
            return tuple(_clone(v) for v in o)
        return o
    return _clone(obj)