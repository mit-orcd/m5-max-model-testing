def clone(obj):
    visited = {}

    def _clone(o):
        if id(o) in visited:
            return visited[id(o)]
        if isinstance(o, dict):
            new = {}
            visited[id(o)] = new
            for k, v in o.items():
                new[_clone(k)] = _clone(v)
            return new
        elif isinstance(o, (list, tuple)):
            new = list if isinstance(o, list) else tuple
            new = new()
            visited[id(o)] = new
            for v in o:
                new.append(_clone(v))
            return new
        else:
            return o

    return _clone(obj)