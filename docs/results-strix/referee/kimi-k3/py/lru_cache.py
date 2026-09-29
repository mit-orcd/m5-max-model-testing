class LRUCache:
    def __init__(self, capacity):
        self.cap = capacity
        self.d = {}
        self.order = []  # oldest first

    def _touch(self, key):
        self.order.remove(key)
        self.order.append(key)

    def get(self, key, default=None):
        if key not in self.d:
            return default
        self._touch(key)
        return self.d[key]

    def put(self, key, value):
        if key in self.d:
            self._touch(key)
        else:
            self.order.append(key)
            if len(self.order) > self.cap:
                del self.d[self.order.pop(0)]
        self.d[key] = value
