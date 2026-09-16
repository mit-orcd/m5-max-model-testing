def clone(obj):
    seen = {}

    def _clone(o):
        if id(o) in seen:
            return seen[id(o)]
        if isinstance(o, dict):
            result = {}
            seen[id(o)] = result
            for k, v in o.items():
                result[_clone(k)] = _clone(v)
            return result
        elif isinstance(o, (list, tuple)):
            result = [_clone(item) for item in o]
            seen[id(o)] = result
            return type(o)(result)
        else:
            return o

    return _clone(obj)