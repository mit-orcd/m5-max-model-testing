class LRUCache:
    def __init__(self, capacity: int):
        if capacity < 1:
            raise ValueError("Capacity must be >= 1")
        self.capacity = capacity
        self.cache = {}
        self.doubly_linked_list = DoublyLinkedList()

    def get(self, key: str, default: any = None) -> any:
        if key in self.cache:
            self.doubly_linked_list.move_to_head(key)
            return self.cache[key]
        return default

    def put(self, key: str, value: any) -> None:
        if key in self.cache:
            self.doubly_linked_list.move_to_head(key)
        elif len(self.cache) >= self.capacity:
            evicted_key = self.doubly_linked_list.remove_tail()
            del self.cache[evicted_key]
        self.cache[key] = value
        self.doubly_linked_list.insert_head(key)

class DoublyLinkedList:
    def __init__(self):
        self.head = Node(None, None)
        self.tail = Node(None, self.head)
        self.head.next = self.tail
        self.head.prev = self.tail.prev = self.head

    def insert_head(self, key: str):
        new_node = Node(key, self.head)
        new_node.next = self.head.next
        self.head.next.prev = new_node
        self.head.next = new_node

    def remove_tail(self) -> str:
        tail_prev = self.tail.prev
        tail_prev.next = self.tail.next
        self.tail.next.prev = tail_prev
        return self.tail.prev.key

    def move_to_head(self, key: str):
        node = self.get_node(key)
        node.remove()
        self.insert_head(key)

    def get_node(self, key: str) -> Node:
        current = self.head
        while current:
            if current.key == key:
                return current
            current = current.next
        raise KeyError(f"Key '{key}' not found in cache.")

    def remove(self, node: Node) -> None:
        node.prev.next = node.next
        node.next.prev = node.prev

class Node:
    def __init__(self, key: str, next: Node):
        self.key = key
        self.next = next
        self.prev = None