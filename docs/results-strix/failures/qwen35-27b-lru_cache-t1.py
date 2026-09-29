class LRUCache:
    def __init__(self, capacity: int):
        if capacity < 1:
            raise ValueError("Capacity must be at least 1")
        self.capacity = capacity
        self.cache = {}
        self.head = None
        self.tail = None

    def _move_to_head(self, node):
        if self.head == node or self.tail == node:
            return
        if node.prev:
            node.prev.next = node.next
        if node.next:
            node.next.prev = node.prev
        if self.tail == node:
            self.tail = node.prev
        self._add_to_head(node)

    def _add_to_head(self, node):
        node.prev = None
        node.next = self.head
        if self.head:
            self.head.prev = node
        self.head = node
        if not self.tail:
            self.tail = node

    def _remove_tail(self):
        if not self.tail:
            return None
        node = self.tail
        if self.tail.prev:
            self.tail.prev.next = None
        else:
            self.head = None
        self.tail = self.tail.prev
        return node

    def get(self, key, default=None):
        if key not in self.cache:
            return default
        node = self.cache[key]
        self._move_to_head(node)
        return node.value

    def put(self, key, value):
        if key in self.cache:
            node = self.cache[key]
            node.value = value
            self._move_to_head(node)
            return
        if len(self.cache) >= self.capacity:
            removed_node = self._remove_tail()
            if removed_node:
                del self.cache[removed_node.key]
        new_node = Node(key, value)
        self.cache[key] = new_node
        self._add_to_head(new_node)


class Node:
    def __init__(self, key, value):
        self.key = key
        self.value = value
        self.prev = None
        self.next = None