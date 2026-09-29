```python
from collections import OrderedDict


class LRUCache:
    def __init__(self, capacity):
        if capacity < 1:
            raise ValueError("capacity must be >= 1")
        self.capacity = capacity
        self._cache = OrderedDict()

    def get(self, key, default=None):
        if key in self._cache:
            self