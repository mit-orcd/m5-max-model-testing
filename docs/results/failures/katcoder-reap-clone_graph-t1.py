def clone(obj):
    seen = {}

    def _clone(obj):
        typ = type(obj)
        if typ in (int, float, bool, str, bytes, type(None)):
            return obj
        if isinstance(obj, tuple):
            return tuple(_clone(item) for item in obj)
        if isinstance(obj, list):
            if obj in seen:
                return seen[obj]
            new = []
            seen[obj] = new
            for item in obj:
                new.append(_clone(item))
            return new
        if isinstance(obj, dict):
            if obj in seen:
                return seen[obj]
            new = {}
            seen[obj] = new
            for k, v in obj.items():
                new[_clone(k)] = _clone(v)
            return new
        if isinstance(obj, set):
            if obj in seen:
                return seen[obj]
            new = set()
            seen[obj] = new
            for item in obj:
                new.add(_clone(item))
            return new
        if hasattr(obj, '__dict__'):
            if obj in seen:
                return seen[obj]
            new = type(obj).__new__(type(obj))
            seen[obj] = new
            for k, v in obj.__dict__.items():
                setattr(new, k, _clone(v))
            return new
        return obj

    return _clone(obj)