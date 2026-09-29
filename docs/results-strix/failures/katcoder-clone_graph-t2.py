def clone(obj):
    seen = {}

    def _clone(obj):
        typ = type(obj)
        if typ in (int, float, str, bool, type(None), bytes, tuple):
            return obj
        if typ is dict:
            if obj in seen:
                return seen[obj]
            new = {}
            seen[obj] = new
            for k, v in obj.items():
                new[k] = _clone(v)
            return new
        if typ is list:
            if obj in seen:
                return seen[obj]
            new = []
            seen[obj] = new
            for v in obj:
                new.append(_clone(v))
            return new
        if typ is set:
            if obj in seen:
                return seen[obj]
            new = set()
            seen[obj] = new
            for v in obj:
                new.add(_clone(v))
            return new
        return obj

    return _clone(obj)