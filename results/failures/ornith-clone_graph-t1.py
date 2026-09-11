def clone(obj):
    id_map = {}

    def _clone(obj):
        if isinstance(obj, dict):
            if id(obj) in id_map:
                return id_map[id(obj)]
            new = {}
            id_map[id(obj)] = new
            for k, v in obj.items():
                new[k] = _clone(v)
            return new
        elif isinstance(obj, (list, tuple)):
            if id(obj) in id_map:
                return id_map[id(obj)]
            new = type(obj)([None] * len(obj))
            id_map[id(obj)] = new
            for i, v in enumerate(obj):
                new[i] = _clone(v)
            return new
        else:
            return obj

    return _clone(obj)