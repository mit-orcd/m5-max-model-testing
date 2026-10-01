```python
def json_diff(a, b):
    """
    Compares two JSON-like structures and returns a sorted list of dotted key paths
    where they differ.
    
    Args:
        a: First JSON-like structure (dict, list, or scalar)
        b: Second JSON-like structure (dict, list, or scalar)
    
    Returns:
        Sorted list of dotted key paths where differences exist.
        Empty list means identical.
    """
    def _diff(a, b, path):
        """
        Recursively compare two values and yield paths where they differ.
        """
        # If both are dicts
        if isinstance(a, dict) and isinstance(b, dict):
            all_keys = set(a.keys()) | set(b.keys())
            for key in all_keys:
                new_path = path + [str(key)] if path else [str(key)]
                if key not in a:
                    # Key only in b
                    yield '.'.join(new_path)
                elif key not in b:
                    # Key only in a
                    yield '.'.join(new_path)
                else:
                    # Both have the key, recurse
                    yield from _diff(a[key], b[key], new_path)
        # If both are lists
        elif isinstance(a, list) and isinstance(b, list):
            if a != b:
                yield '.'.join(path) if path else ''
        # If types differ or one is not dict/list (scalar comparison)
        else:
            if a != b:
                yield '.'.join(path) if path else ''
    
    # Start with empty path
    paths = list(_diff(a, b, []))
    
    # Filter out empty strings if any (shouldn't happen with proper handling)
    # Actually, if path is empty and they differ at root, we need to handle that.
    # Let's reconsider: if path is empty and they differ, what path do we report?
    # The problem says "dotted key paths". For root-level differences, perhaps we report '' or handle differently.
    # Looking at the spec: "keys present in only one side" - these are dict keys.
    # "changed scalar/list values" - these are values at some path.
    # If the root itself is different (e.g., a=1, b=2), what's the path?
    # Let me re-read: "Returns a sorted list of dotted key paths where they differ"
    # For root-level scalar difference, there's no key path. But the spec says empty list means identical.
    # So if a=1, b=2, they differ, so we need to report something.
    # Perhaps we report an empty string or handle root differently.
    
    # Let me reconsider the approach. The paths should be relative to the root.
    # If root values differ and are scalars/lists, we might report '' or handle it.
    # But typically in such diff functions, root-level differences might not have a path.
    # Let me check: if a and b are both dicts, we iterate keys. If they're scalars/lists at root, 
    # and they differ, we need to report that difference somehow.
    
    # Actually, looking more carefully: the function compares structures. If the entire structures 
    # are different scalars/lists, we should report that. But what path? 
    # Perhaps we should not yield empty paths. Let me think about what makes sense.
    
    # If a = {"x": 1} and b = {"x": 2}, path is "x"
    # If a = {"x": 1} and b = {"y": 1}, paths are "x" and "y"
    # If a = [1, 2] and b = [1, 3], they differ as whole lists. What path? 
    # The spec says "lists and scalars are compared as whole values". So if the root is a list 
    # and it differs, we need to report it. But there's no key path for the root.
    
    # I think for root-level differences where there's no key, we might report an empty string 
    # or perhaps the convention is different. Let me look at the requirement again: 
    # "Empty list means identical." So if they differ, we return non-empty.
    
    # Let me adjust: if path is empty and values differ, we yield '' (empty string).
    # But that might be odd. Alternatively, maybe we don't need to handle root-level 
    # scalar/list differences specially if the caller always passes dicts.
    
    # Let me just implement it straightforwardly and see.