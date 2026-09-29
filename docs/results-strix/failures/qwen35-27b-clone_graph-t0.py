def clone(obj):
    memo = {}

    def _clone(o):
        if o is None:
            return None
        if isinstance(o, (int, float, str, bool, type(None))):
            return o
        if id(o) in memo:
            return memo[id(o)]

        if isinstance(o, dict):
            memo[id(o)] = {}
            return {k: _clone(v) for k, v in o.items()}
        if isinstance(o, list):
            memo[id(o)] = []
            result = []
            for item in o:
                result.append(_clone(item))
            return result
        if isinstance(o, tuple):
            memo[id(o)] = tuple(_clone(item) for item in o)
            return memo[id(o)]

        # For other types, return as is (assuming scalars or immutable)
        return o

    return _clone(obj)