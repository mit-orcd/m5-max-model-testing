```python
from collections import deque, defaultdict

def topological_sort(deps):
    # Collect all unique nodes (keys and dependencies)
    nodes = set(deps.keys())
    for dep_list in deps.values():
        nodes.update(dep_list)
    nodes = list(nodes)
    
    # Initialize adjacency list and in-degree dictionary
    adj = defaultdict(list)
    in_degree = {node: 0 for node in nodes}
    
    # Build graph and in-degree counts
    for node, dependencies in deps.items():
        in_degree[node] = len(dependencies)
        for dep in dependencies:
            adj[dep].append(node)