def clone(obj):
    memo = {}

    def _clone(o):
        if id(o) in memo:
            return memo[id(o)]
        if isinstance(o, dict):
            res = {}
            memo[id(o)] = res
            for k, v in o.items():
                res[_clone(k)] = _clone(v)
            return res
        elif isinstance(o, (list, tuple)):
            res = [] if isinstance(o, list) else ()
            memo[id(o)] = res
            for v in o:
                res.append(_clone(v))
            return res
        else:
            return o

    return _clone(obj)