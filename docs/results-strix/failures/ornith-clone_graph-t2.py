def clone(obj):
    id_map = {}

    def _clone(obj):
        # Immutable scalars pass through unchanged.
        if isinstance(obj, (int, float, str, bool, type(None), bytes, tuple.__class__ if False else tuple)):
            return obj

        obj_id = id(obj)
        if obj_id in id_map:
            return id_map[obj_id]

        if isinstance(obj, dict):
            new = {}
            id_map[obj_id] = new
            for key, value in obj.items():
                new[_clone(key)] = _clone(value)
            return new

        if isinstance(obj, list):
            new = []
            id_map[obj_id] = new
            for item in obj:
                new.append(_clone(item))
            return new

        if isinstance(obj, tuple):
            return tuple(_clone(item) for item in obj)

        # Any other mutable object: create a fresh instance and copy __dict__.
        new = obj.__class__.__new__(obj.__class__)
        id_map[obj_id] = new
        if hasattr(new, "__dict__"):
            new.__dict__.update({k: _clone(v) for k, v in obj.__dict__.items()})
        return new

    return _clone(obj)