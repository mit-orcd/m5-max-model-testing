```python
class LRUCache:
    class _Node:
        __slots__ = ('key', 'value', 'prev', 'next')
        def __init__(self, key=None, value=None):
            self.key = key
            self.value = value
            self.prev = None
            self.next = None

    def __init__(self, capacity):
        self.capacity = capacity
        self.size = 0
        self.map = {}
        self._head = self._Node()
        self._tail = self._Node()
        self._head.next = self._tail
        self._tail.prev = self._head

    def _remove(self, node):
        node.prev.next = node.next
        node.next.prev = node.prev

    def _add_front(self, node):
        node.prev = self._head
        node.next = self._head.next
        self._head.next.prev = node
        self._head.next = node

    def _move_to_front(self, node):
        self._remove(node)
        self._add_front(node)

    def get(self, key, default=None):
        if key in self.map:
            node =