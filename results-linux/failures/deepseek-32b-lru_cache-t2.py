class LRUCache:
    def __init__(self, capacity):
        self.capacity = capacity
        self.cache = {}

    def get(self, key, default=None):
        if key in self.cache:
            value = self.cache[key]
            self.cache.move_to_end(key)
            return value
        return default

    def put(self, key, value):
        if key in self.cache:
            self.cache[key] = value
            self.cache.move_to_end(key)
        else:
            self.cache[key] = value
            if len(self.cache) > self.capacity:
                # Remove the first item (least recently used)
                lru_key = next(iter(self.cache))
                del self.cache[lru_key]