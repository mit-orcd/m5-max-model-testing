def clone(obj):
    # Map to keep track of original objects and their clones to handle cycles and shared refs
    memo = {}
    
    def _clone_recursive(item):
        # Check if the item has already been cloned (handles cycles and shared refs)
        if item in memo:
            return memo[item]
        
        # Handle scalar types (immutable): return directly
        # We consider basic types as scalars: None, bool, int, float, complex, str, bytes, frozenset
        # We also handle tuples (which are immutable but need cloning if containing mutable items)
        # However, the requirement says "tuples" are part of the structure. 
        # Since tuples are immutable, we usually return them as is, BUT if they contain mutable objects,
        # we must ensure those are deep copied. Wait, tuples themselves cannot be modified, so sharing is fine 
        # EXCEPT if the requirement implies we need to ensure NO mutable object is shared.
        # If we return the original tuple, does it violate "no mutable object may be shared"?
        # No, because the tuple is immutable. But if the tuple contains a mutable object, that object is shared.
        # So we must reconstruct the tuple with cloned contents.
        
        # Identify immutable scalars that don't need cloning
        if type(item) in (type(None), bool, int, float, complex, str, bytes, frozenset):
            return item
        
        # Handle Lists
        if isinstance(item, list):
            # Create the new list but don't add to memo yet (to handle cycles inside)
            new_list = []
            memo[item] = new_list
            for element in item:
                new_list.append(_clone_recursive(element))
            return new_list
        
        # Handle Tuples
        if isinstance(item, tuple):
            # Create the new tuple
            new_tuple = tuple(_clone_recursive(element) for element in item)
            return new_tuple
        
        # Handle Dictionaries
        if isinstance(item, dict):
            # Create the new dict but don't add to memo yet (to handle cycles inside)
            new_dict = {}
            memo[item] = new_dict
            for key, value in item.items():
                new_dict[key] = _clone_recursive(value)
            return new_dict
        
        # For any other type (e.g., custom objects, sets, etc.)
        # If it's a mutable set, we clone it. If immutable, return as is.
        # Since the prompt specifies dicts, lists, tuples, and scalars, we assume standard types.
        # If a custom object has __dict__, we might need to clone attributes, but the prompt implies 
        # only the structure of dicts/lists/tuples/scalars. 
        # However, to be safe and handle "cycles" in general objects, we treat unknown objects 
        # as not needing deep copy unless they are mutable containers.
        # If it's a set, it's mutable.
        if isinstance(item, set):
            new_set = set()
            memo[item] = new_set
            for element in item:
                new_set.add(_clone_recursive(element))
            return new_set
        
        # If it's a custom mutable object, we can't deep copy it generically without knowing its structure.
        # The prompt says "structure of dicts, lists, tuples and scalars". 
        # We assume inputs are composed of these. If an object is found that is not a scalar 
        # and not list/dict/tuple/set, we return it as is (assuming it's immutable or needs no cloning).
        return item

    return _clone_recursive(obj)