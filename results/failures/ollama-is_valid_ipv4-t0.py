```python
def is_valid_ipv4(s):
    parts = s.split('.')
    if len(parts) != 4:
        return False
    for part in parts:
        if not part or not all(c in '0123456789' for c in part):
            return False
        if len(part) > 1 and part[0] == '0':
            return False