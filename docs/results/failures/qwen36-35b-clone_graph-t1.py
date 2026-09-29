def clone(obj):
    memo = {}

    def _clone(o):
        if isinstance(o, dict):
            if id(o) not in memo:
                memo[id(o)] = {key: _clone(val) for key, val in o.items()}
            return memo[id(o)]
        elif isinstance(o, list):
            if id(o) not in memo:
                memo[id(o)] = [_clone(item) for item in o]
            return memo[id(o)]
        elif isinstance(o, tuple):
            return tuple(_clone(item) for item in o)
        else:
            return o

    return _clone(obj)