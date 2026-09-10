```python
def is_valid_ipv4(s):
    parts = s.split('.')
    if len(parts) != 4:
        return False
    for part in parts:
        if not part.isdigit