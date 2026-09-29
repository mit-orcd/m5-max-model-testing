from collections import deque

def topological_sort(deps):
    # Create a set of all nodes mentioned in the graph
    nodes = set()
    for node, dependents in deps.items():
        nodes.add(node)
        nodes.update(dependents0)
    
    # Calculate in-degree for all nodes
    in_degree = {node: 0 for node in nodes}
    for dependents in deps.values():
        for node in dependents:
            in_degree[node] += 1
    
    # Initialize a queue with nodes having in-degree 0
    queue = deque([node for node in nodes if in_degree[node] == 0])
    topo_order = []
    
    # Process nodes
    while queue:
        current = queue.popleft()
        topo_order.append(current)
        
        # Check if current is a key in deps (it might not be, if it's only a dependency)
        if current in deps:
            for neighbor in deps[current]:
                in_degree[neighbor] -= 1
                if in_degree[neighbor] == 0:
                    queue.append(neighbor)
    
    # Check for cycles: if we couldn't process all nodes, there is a cycle
    if len(topo_order) != len(nodes):
        return None
    
    return topo_order