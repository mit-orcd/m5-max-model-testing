def clone(obj):
    memo = {}
    
    def _clone(item):
        # Handle immutable scalars and None
        if item is None or isinstance(item, (int, float, bool, str, tuple, frozenset)):
            return item
        
        # If already cloned, return the reference to the clone
        if item in memo:
            return memo[item]
        
        # Create a new empty container for the current item to establish a cycle reference
        # We must do this before recursing to handle cycles
        if isinstance(item, list):
            new_item = []
        elif isinstance(item, dict):
            new_item = {}
        elif isinstance(item, tuple):
            # Tuples are immutable, but if they contain mutable objects, we need to clone those.
            # However, the tuple itself is returned as-is if it only contains immutable or already cloned items.
            # But to strictly follow "no mutable object may be shared", we reconstruct the tuple.
            # Since tuples are immutable, we can't modify them in place, so we create a new tuple.
            # But we must handle the case where the tuple contains itself.
            # We can't put a new empty tuple in memo and then fill it later (tuples are immutable).
            # So for tuples containing cycles, we just recurse and hope the recursion handles it,
            # but we need to ensure if the tuple is in the memo, we return it.
            # Actually, since tuples are immutable, a self-referential tuple is rare and complex to reconstruct
            # without side effects. But the problem says "structure of dicts, lists, tuples".
            # Let's treat tuples as immutable values unless they contain mutable references.
            # If a tuple contains a list that points back to the tuple, that list is mutable.
            # We can clone the tuple by cloning its elements.
            # The cycle detection in the mutable children will handle the reference back to the tuple.
            # But we need to ensure the tuple itself isn't shared.
            return tuple(_clone(x) for x in item)
        else:
            # Fallback for unknown types or other mutable types not specified
            # Assuming the problem only implies dicts, lists, tuples, and scalars.
            # If a custom mutable object is passed, we can't clone it deeply without knowing its structure.
            # We will treat it as a scalar or raise an error, but the prompt implies standard types.
            # Let's assume standard types only. If it's a set, we treat it like a tuple (immutable container).
            if isinstance(item, set):
                return set(_clone(x) for x in item)
            # If it's an object instance, we can't deep copy it generically without copy module or __deepcopy__.
            # Given the constraints, we assume only dict, list, tuple, scalar.
            return item
        
        # Register the new empty container in memo BEFORE recursing to handle cycles
        memo[item] = new_item
        
        # Fill the container
        if isinstance(item, dict):
            for k, v in item.items():
                # Keys are immutable (usually), but if they are mutable, we clone them too?
                # Standard dict keys are immutable. We assume keys are immutable or need cloning if mutable.
                # If keys are mutable, we must clone them to avoid sharing.
                new_k = _clone(k)
                new_v = _clone(v)
                new_item[new_k] = new_v
        elif isinstance(item, list):
            for val in item:
                new_item.append(_clone(val))
        elif isinstance(item, set):
            for val in item:
                new_item.add(_clone(val))
        
        return new_item

    return _clone(obj)