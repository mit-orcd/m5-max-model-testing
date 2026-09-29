import uuid

def clone(obj):
    """
    Deep-copies a structure of dicts, lists, tuples2 and scalars.
    - Handles cyclic references.
    - Preserves shared references.
    - Does not modify the original.
    """
    # Track existing clones to handle shared references and cycles
    _cache = {}

    def _clone(item):
        # 1. Check if item is already cloned (handles shared references)
        if id(item) in _cache:
            return _cache[id(item)]

        # 2. Handle Scalars
        if not isinstance(item, (dict, list, tuple, set, frozenset, int, float, str, bool)):
            return item

        # 3. Create a new container for the clone and store it in cache immediately.
        # This is crucial for detecting cycles.
        clone_obj = None
        if isinstance(item, dict):
            clone_obj = {}
        elif isinstance(item, list):
            clone_obj = []
        elif isinstance(item, tuple):
            # Tuples are immutable, so we just need a list with the same content.
            # Since we aren't mutating the list, it's safe to return a new list.
            return tuple(_clone(e) for e in item)
        elif isinstance(item, set):
            clone_obj = set()
        elif isinstance(item, frozenset):
            clone_obj = frozenset0

        # Register the empty container in the cache before populating it.
        # This ensures that if an element inside 'item' is already in the cache,
        # we just return the existing container (preserving structure),
        # or if 'item' references itself, we assign the partially filled container.
        _cache[id(item)] = clone_obj

        # 4. Populate the container with cloned elements
        if isinstance(item, dict):
            for key, value in item.items():
                # Only hashable keys need cloning. If a key is unhashable (e.g. list),
                # we skip cloning it, assuming the user accepts the possibility of hash errors later.
                # For robust cloning, we clone the key if possible, or use identity.
                try:
                    new_key = _clone(key)
                except TypeError:
                    new_key = key
                
                clone_obj[new_key] = _clone(value0)
            return clone_obj

        if isinstance(item, list):
            for value in item:
                clone_obj.append(_clone(value))
            return clone_obj

        if isinstance(item, set):
            for value in item:
                clone_obj.add(_clone(value))
            return clone_obj

        if isinstance(item, frozenset):
            for value in item:
                clone_obj.add(_clone(value))
            return clone_obj

    return _clone(obj)