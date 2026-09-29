To solve this problem, we need to implement a topological sort function that takes a dictionary of dependencies and returns a list of nodes in an order where each node comes after its dependencies. If there is a cycle in the graph, the function should return `None`.

### Approach
The approach we use is based on Kahn's algorithm, which is efficient for topological sorting and also helps in detecting cycles in the graph. Here are the steps involved:

1. **Collect All Nodes**: Extract all unique nodes from the given dependencies. This includes nodes that are only mentioned as dependencies and not as keys in the dictionary.

2. **Build Adjacency List and In-Degree Count**: Construct an adjacency list to represent the graph and an in-degree count for each node. The adjacency list helps in tracking the neighbors of each node, while the in-degree count helps in determining when a node can be processed.

3. **Initialize Queue with Zero In-Degree Nodes**: Start with nodes that have zero in-degree, as these nodes have no dependencies and can be processed first.

4. **Process Nodes Using Queue**: Use a queue to process each node. For each node processed, add it to the result list and decrement the in-degree count of its neighbors. If any neighbor's in-degree becomes zero, enqueue it.

5. **Check for Cycles**: After processing all nodes, if the result list does not contain all nodes, it indicates a cycle in the graph, and we return `None`.

### Solution Code
```python
from collections import deque

def topological_sort(deps):
    if not deps:
        return []
    
    all_nodes = set(deps.keys())
    for deps_list in deps.values():
        all_nodes.update(deps_list)
    all_nodes = list(all_nodes)
    
    in_degree = {node: 0 for node in all_nodes}
    adj = {node: [] for node in all_nodes}
    
    for u in deps:
        for v in deps[u]:
            adj[v].append(u)
            in_degree[u] += 1
    
    queue = deque()
    for node in all_nodes:
        if in_degree[node] == 0:
            queue.append(node)
    
    result = []
    while queue:
        node = queue.popleft()
        result.append(node)
        for neighbor in adj[node]:
            in_degree[neighbor] -=