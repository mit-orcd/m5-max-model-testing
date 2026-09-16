class LRUCache:
    def __init__(self, capacity: int):
        if capacity < 1:
            raise ValueError("Capacity must be at least 1")
        self.capacity = capacity
        self.cache = {}
        self.head = None  # Most recently used
        self.tail = None  # Least recently used

    def _add_to_head(self, node):
        """Moves a node to the head (MRU)."""
        if self.head is None:
            self.head = self.tail = node
            node.prev = node.next = None
            return

        node.prev = None
        node.next = self.head
        self.head.prev = node
        self.head = node

    def _remove_node(self, node):
        """Removes a node from the doubly linked list."""
        if node.prev:
            node.prev.next = node.next
        else:
            self.head = node.next

        if node.next:
            node.next.prev = node.prev
        else:
            self.tail = node.prev

        node.prev = node.next = None

    def _evict_tail(self):
        """Removes the least recently used item (tail)."""
        if self.tail:
            self._remove_node(self.tail)

    def get(self, key, default=None):
        if key not in self.cache:
            return default

        node = self.cache[key]
        self._add_to_head(node)
        return node.value

    def put(self, key, value):
        if key in self.cache:
            # Update existing key
            node = self.cache[key]
            node.value = value
            self._remove_node(node)
            self._add_to_head(node)
        else:
            # Insert new key
            if len(self.cache) == self.capacity:
                # Evict LRU item
                self._evict_tail()
                if self.tail and self.tail in self.cache:
                    del self.cache[self.tail.key]

            # Create new node
            node = type('Node', (), {'key': key, 'value': value, 'prev': None, 'next': None})()
            self.cache[key] = node
            self._add_to_head(node)