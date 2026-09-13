def clone(obj):
    id_map = {}
    
    def _clone(o):
        # Handle immutable types (scalars) directly
        if isinstance(o, (type(None), bool, int, float, str, bytes, tuple)):
            return o
        
        # Check if we've already cloned this object (handles cycles and shared refs)
        oid = id(o)
        if oid in id_map:
            return id_map[oid]
        
        # Create a new container
        if isinstance(o, dict):
            new_o = {}
            id_map[oid] = new_o
            for k, v in o.items():
                new_o[clone(k)] = _clone(v)
        elif isinstance(o, list):
            new_o = []
            id_map[oid] = new_o
            for item in o:
                new_o.append(_clone(item))
        elif isinstance(o, set):
            new_o = set()
            id_map[oid] = new_o
            for item in o:
                new_o.add(_clone(item))
        else:
            # Fallback for other types: try to use copy semantics or return as is
            # For standard library collections not explicitly handled, deep copy via recursion might be tricky.
            # However, the prompt specifies dicts, lists, tuples and scalars.
            # Tuples are immutable, handled above.
            # If it's a custom object, we can't deep copy it without knowing its structure.
            # Assuming standard types only as per "structure of dicts, lists, tuples and scalars".
            new_o = o
        return new_o
    
    return _clone(obj)