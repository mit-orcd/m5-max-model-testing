To solve this problem, we need to implement a fixed-capacity Least Recently Used (LRU) cache. The LRU cache efficiently manages memory by evicting the least recently used items when the cache is full. The cache should support two main operations: `get` and `put`. The `get` operation retrieves an item and marks it as the most recently used, while the `put` operation inserts or updates an item and evicts the least recently used item if the cache exceeds its capacity.

### Approach
The LRU cache is implemented using a combination of a dictionary and a doubly linked list. The dictionary stores key-value pairs for quick lookups, while the doubly linked list keeps track of the order of usage to determine which item is the least recently used.

1. **Node Structure**: Each node in the linked list contains a key, value, and pointers to the previous and next nodes. This helps in efficiently moving nodes to the front (most recently used) and removing nodes from the end (least recently used).

2. **Dummy Nodes**: We use dummy head and tail nodes to simplify the linked list operations. This avoids special cases for the head and tail nodes when performing insertions and deletions.

3. **Move to Head**: When an item is accessed (either via `get` or `put`), it is moved to the front of the linked list to mark it as the most recently used.

4. **Eviction**: When the cache is full and a new item is added, the least recently used item (at the tail of the linked list) is evicted.

### Solution Code
```python
class LRUCache:
    class Node:
        def __init__(self, key, value):
            self.key = key
            self.value = value
            self.prev = None
            self.next = None

    def __init__(self, capacity):
        self.capacity = capacity
        self.cache = {}  # Maps keys to Node objects
        self.head = LRUCache.Node(None, None)  # Dummy head node
        self.tail = LRUCache.Node(None, None)  # Dummy tail node
        self.head.next = self.tail
        self.tail.prev = self.head
        self.size = 0

    def get(self, key, default=None):
        if key in self.cache:
            node = self.cache[key]
            self._move_to_head(node)
            return node.value
        return default

    def put(self, key, value):
        if key in self.cache:
            node = self.cache[key]
            node.value = value
            self._move_to_head(node)
        else:
            new_node = LRUCache.Node(key, value)
            self.cache[key] = new_node
            self._add_to_head(new_node)
            self.size += 1
            if self.size > self.capacity:
                removed_node = self._remove_tail()
                del self.cache[removed_node.key]
                self.size -= 1

    def _move_to_head(self, node):
        # Unlink the node from its current position
        prev_node = node.prev
        next_node = node.next
        prev_node.next = next_node
        if next_node is not None:
            next_node.prev = prev_node
        # Link the node to the head
        node.prev = self.head
        node.next = self.head.next
        self.head.next.prev = node
        self.head.next = node

    def _add_to_head(self, node):
        # Insert node after the head
        node.prev = self.head
        node.next = self.head.next
        self.head.next.prev = node
        self.head.next = node

    def _remove_tail(self):
        # Remove the node before the tail
        node = self.tail.prev
        # Unlink the node
        node.prev.next = self.tail
        self.tail.prev = node.prev
        return node