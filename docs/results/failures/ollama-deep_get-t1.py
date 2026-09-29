```python
def deep_get(d, path, default=None):
    keys = path.split('.')
    current = d
    for key in keys:
        if not isinstance(current, dict) or key not in current:
            return default
        current = current[key]
    return current