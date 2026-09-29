def clone(obj):
    def _clone(obj, memo):
        if isinstance(obj, dict):
            result = {}
            memo[id(obj)] = result
            for key, value in obj.items():
                result[_clone(key, memo)] = _clone(value, memo)
            return result
        elif isinstance(obj, list):
            result = []
            memo[id(obj)] = result
            for value in obj:
                result.append(_clone(value, memo))
            return result
        elif isinstance(obj, tuple):
            result = []
            memo[id(obj)] = result
            for value in obj:
                result.append(_clone(value, memo))
            result = tuple(result)
            memo[id(obj)] = result
            return result
        elif isinstance(obj, (int, float, str, bool)) or obj is None:
            return obj
        else:
            return obj

    memo = {}
    return _clone(obj, memo)