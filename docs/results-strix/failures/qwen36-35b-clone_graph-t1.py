def clone(obj):
    import sys
    
    # Use id mapping to handle cycles and shared references
    # memo maps id(original_object) -> (cloned_object, placeholder_replaced_flag)
    # However, we need to handle the case where we are currently cloning an object
    # that refers to itself.
    
    memo = {}
    # We also need to track objects currently being constructed to detect cycles
    # But a simpler approach for cycles: if we encounter an object already in memo,
    # return the cloned version.
    
    # Problem: If we return the cloned version before it's fully constructed,
    # we might have incomplete data. But since we build the structure top-down,
    # when we encounter a cycle, we just need to return the reference to the
    # new object being built.
    
    # Strategy:
    # 1. Check if obj is in memo. If so, return memo[obj].
    # 2. Create a placeholder for mutable objects (dicts, lists) in memo BEFORE recursing.
    #    For tuples and scalars, we just clone/return immediately.
    
    return _clone_impl(obj, memo)


def _clone_impl(obj, memo):
    # Handle immutable types: int, float, str, bool, None, tuple (if we handle tuples carefully)
    # Note: Tuples are immutable, but their contents might be mutable.
    # However, the problem says "shared references must stay shared".
    # If a tuple contains the same list object twice, the clone should have a tuple with the same cloned list twice.
    
    # Check memo first
    oid = id(obj)
    if oid in memo:
        return memo[oid]
    
    if isinstance(obj, dict):
        # Create a new dict and put a placeholder in memo immediately
        new_dict = {}
        memo[oid] = new_dict
        for k, v in obj.items():
            new_dict[k] = _clone_impl(k, memo)  # Keys should also be cloned? 
            # Usually in deep copy, keys are cloned if they are mutable.
            # But dict keys are typically immutable. Let's clone them to be safe and consistent.
            new_dict[new_dict.__class__] # Just to ensure syntax is correct, we use new_dict
            # Actually, let's rebuild the dict properly
            pass
        
        # Let's restart the dict logic cleanly
        del memo[oid] # Remove placeholder, we'll fill it properly
        new_dict = {}
        memo[oid] = new_dict # Re-add
        
        for k, v in obj.items():
            ck = _clone_impl(k, memo)
            cv = _clone_impl(v, memo)
            new_dict[ck] = cv
        return new_dict

    elif isinstance(obj, list):
        new_list = []
        memo[oid] = new_list
        for item in obj:
            new_list.append(_clone_impl(item, memo))
        return new_list

    elif isinstance(obj, tuple):
        # Tuples are immutable, but we need to deep copy their contents
        # and preserve shared references among elements
        new_tuple = tuple(_clone_impl(item, memo) for item in obj)
        return new_tuple

    else:
        # Scalars and other immutable types
        return obj