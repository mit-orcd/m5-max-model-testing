from collections import deque

def topological_sort(deps):
    nodes = set(deps.keys())
    for dep_list in deps.values():
        nodes.update(dep_list)
    nodes = list(nodes)
    
    adj = {node: [] for node in nodes}
    in_degree = {node: 0 for node in nodes}
    
    for node in nodes:
        dep_list = deps.get(node, [])
        unique_deps = set(dep_list)
        in_degree[node] = len(unique_deps)
        for dep in unique_deps:
            adj[dep].append(node)
    
    queue = deque(node for node in nodes if in_degree[node] == 0)
    result = []
    
    while queue:
        current = queue.popleft()
        result.append(current)
        for neighbor in adj[current]:
            in_degree[neighbor] -= 1
            if in_degree[neighbor] == 0: