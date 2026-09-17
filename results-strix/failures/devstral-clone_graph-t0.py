def clone(obj):
    memo = {}

    def _clone(o):
        if id(o) in memo:
            return memo[id(o)]
        if isinstance(o, dict):
            result = {}
            memo[id(o)] = result
            for k, v in o.items():
                result[_clone(k)] = _clone(v)
            return result
        elif isinstance(o, (list, tuple)):
            result = [] if isinstance(o, list) else ()
            memo[id(o)] = result
            for item in o:
                result.append(_clone(item))
            return result if isinstance(o, list) else tuple(result)
        else:
            return o

    return _clone(obj)