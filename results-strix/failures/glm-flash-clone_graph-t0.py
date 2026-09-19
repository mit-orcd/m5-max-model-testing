def clone(obj):
    """
    Deep-copies a structure of dicts, lists, tuples and scalars without using the copy module.
    Handles cycles and shared references correctly.
    """
    # A registry to keep track of objects we have already cloned.
    # Maps original object -> its clone.
    _registry = {}

    def _clone(obj):
        # Check if we have already cloned this object.
        # This handles shared references and cycles.
        if obj in _registry:
            return _registry[obj]

        # Handle immutable types (int, str, bool, None, etc.) and tuples.
        # Tuples are immutable, so we can safely return them directly.
        # Note: We do not register immutable objects in the registry
        # because they are singletons and cannot be mutated.
        if isinstance(obj, (int, float, str, bool, type(None))):
            return obj
        if isinstance(obj, tuple):
            # Recursively clone the tuple elements.
            # Tuples are immutable, so we don't need to worry about cycles
            # affecting the tuple itself, only its contents.
            cloned_tuple = tuple(_clone(item) for item in obj)
            return cloned_tuple

        # Handle mutable types: dict, list
        if isinstance(obj, dict):
            # Create a new empty dict for the clone.
            cloned_dict = {}
            # Register the new dict in the registry before cloning its contents.
            # This handles cycles where a dict might contain itself.
            _registry[obj] = cloned_dict

            # Clone each key-value pair.
            # Keys must be immutable, so we don't need to clone them.
            # Values are cloned recursively.
            for key, value in obj.items():
                cloned_dict[key] = _clone(value)

            return cloned_dict

        if isinstance(obj, list):
            # Create a new empty list for the clone.
            cloned_list = []
            # Register the new list in the registry before cloning its contents.
            # This handles cycles where a list might contain itself.
            _registry[obj] = cloned_list

            # Clone each item in the list.
            for item in obj:
                cloned_list.append(_clone(item))

            return cloned_list

        # If the object is of an unsupported type, raise an error.
        raise TypeError(f"Cannot clone object of type {type(obj)}")

    return _clone(obj)