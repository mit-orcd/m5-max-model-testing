def clone(obj):
    memo = {}
    
    def _clone(obj):
        obj_id = id(obj)
        
        # Handle immutable scalars and already processed objects
        if isinstance(obj, (int, float, str, bool, type(None), tuple)):
            return obj
        
        # Check if this object was already cloned
        if obj_id in memo:
            return memo[obj_id]
        
        # Handle mutable containers
        if isinstance(obj, list):
            new_obj = []
            memo[obj_id] = new_obj
            for item in obj:
                new_obj.append(_clone(item))
            return new_obj
        
        elif isinstance(obj, dict):
            new_obj = {}
            memo[obj_id] = new_obj
            for key, value in obj.items():
                new_key = _clone(key)
                new_value = _clone(value)
                new_obj[new_key] = new_value
            return new_obj
        
        elif isinstance(obj, tuple):
            # Tuples are immutable but may contain mutable objects
            new_obj = tuple(_clone(item) for item in obj)
            memo[obj_id] = new_obj
            return new_obj
        
        else:
            # For other objects, create a shallow copy and process attributes
            new_obj = obj.__new__(obj.__class__)
            memo[obj_id] = new_obj
            if hasattr(obj, '__dict__'):
                for key, value in obj.__dict__.items():
                    setattr(new_obj, key, _clone(value))
            return new_obj
    
    return _clone(obj)