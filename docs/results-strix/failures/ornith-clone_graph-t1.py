def clone(obj):
    id_map = {}

    def _clone(item):
        oid = id(item)
        if isinstance(item, dict):
            new = {}
            id_map[oid] = new
            for k, v in item.items():
                new[k] = _clone(v)
            return new
        if isinstance(item, (list, tuple)):
            new = []
            id_map[oid] = new
            for v in item:
                new.append(_clone(v))
            return type(item)(new)
        return item

    return _clone(obj)