def clone(obj):
    memo = {}
    
    def _clone(obj):
        # Handle None and scalars (int, float, str, bool, etc.)
        if obj is None or isinstance(obj, (int, float, str, bool, bytes, type(None))):
            return obj
        
        # For objects that might be in memo (already cloned)
        obj_id = id(obj)
        if obj_id in memo:
            return memo[obj_id]
        
        # Handle tuples (immutable, but can contain mutable objects)
        if isinstance(obj, tuple):
            return obj  # Tuples are immutable and hashable, can be reused directly
        
        # Handle dicts
        if isinstance(obj, dict):
            new_dict = {}
            memo[obj_id] = new_dict
            for key, value in obj.items():
                # Keys are typically immutable, but we clone to be safe
                new_key = _clone(key) if isinstance(key, (dict, list, tuple)) else key
                new_value = _clone(value)
                new_dict[new_key] = new_value
            return new_dict
        
        # Handle lists
        if isinstance(obj, list):
            new_list = []
            memo[obj_id] = new_list
            for item in obj:
                new_list.append(_clone(item))
            return new_list
        
        # Handle other types (custom objects, sets, etc.)
        # For unknown types, try to create a new instance without arguments
        # and copy attributes if possible
        try:
            new_obj = obj.__new__(obj.__class__)
            memo[obj_id] = new_obj
            if hasattr(obj, '__dict__'):
                new_obj.__dict__ = _clone(obj.__dict__)
            elif hasattr(obj, '__slots__'):
                for slot in obj.__slots__:
                    if hasattr(obj, slot):
                        setattr(new_obj, slot, _clone(getattr(obj, slot)))
            return new_obj
        except Exception:
            # If we can't clone it, return the object as-is (for immutable types)
            return obj
    
    return _clone(obj)