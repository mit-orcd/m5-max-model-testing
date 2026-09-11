def clone(obj):
    # Dictionary to track already cloned objects to handle cycles and shared references
    seen = {}
    
    def _clone(obj):
        # If object is already cloned, return the clone
        if id(obj) in seen:
            return seen[id(obj)]
        
        # Handle scalars (not dict, list, tuple, set, etc.)
        if not isinstance(obj, (dict, list, tuple, set, frozenset)):
            # For immutable scalars, just return the same object
            return obj
        
        # Create a new container of the same type
        if isinstance(obj, dict):
            new_obj = {}
        elif isinstance(obj, list):
            new_obj = []
        elif isinstance obj:
            new_obj = tuple([_clone(item) for item in obj]) if obj else ()
            return new_obj
        elif isinstance(obj, set):
            new_obj = set()
        elif isinstance(obj, frozenset):
            new_obj = frozenset()
        else:
            new_obj = type(obj)()
        
        # Store the clone before populating to handle cycles
        seen[id(obj)] = new_obj
        
        # Populate the new container
        if isinstance(obj, dict):
            for key, value in obj.items():
                new_key = _clone(key)
                new_val = _clone(value)
                new_obj[new_key] = new_val
        elif isinstance(obj, (list, set)):
            for item in obj:
                new_obj.add(_clone(item)) if isinstance(obj, set) else new_obj.append(_clone(item))
        elif isinstance(obj, frozenset):
            new_obj = frozenset(_clone(item) for item in obj)
        elif isinstance(obj, tuple):
            # Handle tuple specially: convert to list first to avoid issues with empty tuples
            new_obj = tuple(_clone(item) for item in obj)
        
        return new_obj
    
    return _clone(obj)