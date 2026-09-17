def json_diff(a, b):
    """
    Compare two JSON-like structures (dicts, lists, scalars) and return a sorted
    list of dotted key paths where they differ.
    """
    diffs = []

    def _recurse(path, left, right):
        # Helper to add a dotted path to diffs
        def add():
            diffs.append(".".join(path))

        # If both are dicts, recurse into keys
        if isinstance(left, dict) and isinstance(right, dict):
            all_keys = set(left) | set(right)
            for key in all_keys:
                new_path = path + [str(key)]
                if key not in left:
                    add()                     # key missing on left
                elif key not in right:
                    add()                     # key missing on right
                else:
                    _recurse(new_path, left[key], right[key])
            return

        # If types differ (one dict, other not) -> difference at this path
        if isinstance(left, dict) or isinstance(right, dict):
            add()
            return

        # For lists and scalars, compare whole values
        if left != right:
            add()

    _recurse([], a, b)
    return sorted(diffs)