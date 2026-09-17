def clone(obj):
    seen = {}

    def _clone(obj):
        if isinstance(obj, (dict, list, tuple)):
            if id(obj) in seen:
                return seen[id(obj)]
            if isinstance(obj, tuple):
                new_obj = tuple()
            else:
                new_obj = obj.__class__()
            seen[id(obj)] = new_obj
            if isinstance(obj, dict):
                for k, v in obj.items():
                    new_obj[_clone(k)] = _clone(v)
            else:
                for item in obj:
                    new_obj.append(_clone(item))
            return new_obj
        else:
            return obj

    return _clone(obj)