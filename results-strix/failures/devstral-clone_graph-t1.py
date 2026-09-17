def clone(obj):
    memo = {}

    def _clone(obj):
        if id(obj) in memo:
            return memo[id(obj)]

        if isinstance(obj, dict):
            result = {}
            memo[id(obj)] = result
            for key, value in obj.items():
                result[_clone(key)] = _clone(value)
            return result

        elif isinstance(obj, (list, tuple)):
            result = [] if isinstance(obj, list) else ()
            memo[id(obj)] = result
            result = [_clone(item) for item in obj]
            return result if isinstance(obj, list) else tuple(result)

        else:
            return obj

    return _clone(obj)