def clone(obj):
    mapping = {}
    
    def _clone(obj):
        if isinstance(obj, dict):
            if obj in mapping:
                return mapping[obj]
            result = {}
            mapping[obj] = result
            for k, v in obj.items():
                result[k] = _clone(v)
            return result
        elif isinstance(obj, list):
            if obj in mapping:
                return mapping[obj]
            result = []
            mapping[obj] = result
            for item in obj:
                result.append(_clone(item))
            return result
        elif isinstance(obj, tuple):
            if obj in mapping:
                return mapping[obj]
            result = tuple(_clone(item) for item in obj)
            mapping[obj] = result
            return result
        else:
            return obj
    
    return _clone(obj)