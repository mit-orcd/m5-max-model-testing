class LRUCache:
    def __init__(self, capacity):
        if capacity < 1:
            raise ValueError("Capacity must be at least 1")
        self.capacity = capacity
        self.cache = {}

    def get(self, key, default=None):
        if key in self.cache:
            self._move_to_end(key)
        return self.cache.get(key, default)

    def put(self, key, value):
        if key in self.cache:
            self._move_to_end(key)
        
        self.cache[key] = value
        
        if len(self.cache) > self.capacity:
            self._evict_oldest()

    def _move_to_end(self, key):
        self.cache.move_to_end(key)

    def _evict_oldest(self):
        self.cache.popitem(last=False)