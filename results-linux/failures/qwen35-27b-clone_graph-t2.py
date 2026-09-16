def clone(obj):
    memo = {}

    def _clone(val):
        if val is None:
            return None
        if isinstance(val, bool):
            return val
        if isinstance(val, (int, float, str, bytes)):
            return val
        if id(val) in memo:
            return memo[id(val)]

        if isinstance(val, dict):
            memo[id(val)] = {}
            return {k: _clone(v) for k, v in val.items()}
        if isinstance(val, list):
            memo[id(val)] = []
            result = []
            for item in val:
                result.append(_clone(item))
            return result
        if isinstance(val, tuple):
            memo[id(val)] = tuple()
            result = []
            for item in val:
                result.append(_clone(item))
            return tuple(result)

        # For other types, assume they are scalars or immutable
        return val

    return _clone(obj)