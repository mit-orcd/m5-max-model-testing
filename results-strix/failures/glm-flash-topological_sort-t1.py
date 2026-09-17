def topological_sort(deps):
    """
    Performs a topological sort on a dependency graph.
    Returns a list of nodes in topological order, or None if a cycle is detected.
    """
    # Initialize the set of all nodes found in the keys or values of deps
    nodes = set()
    for node, dependents in deps.items():
        nodes.add(node)
        nodes.update(dependents)

    # in_degree maps node -> number of incoming edges
    in_degree = {node: 0 for node in nodes}
    
    # Populate in_degree based on the provided dependencies
    for node, dependencies in deps.items():
        for dep in dependencies:
            in_degree[dep] += 1

    # Initialize queue with all nodes that have no dependencies
    from collections import deque
    queue = deque([node for node in nodes if in_degree[node] == 0])
    
    sorted_order = []
    
    while queue:
        current = queue.popleft()
        sorted_order.append(current)
        
        # For each neighbor (node that depends on current), decrement its in_degree
        # We iterate over a copy of deps.items() to allow safe modification of deps if needed,
        # though in_degree handles the cycle detection logic.
        for neighbor in deps.get(current, []):
            in_degree[neighbor] -= 1
            if in_degree[neighbor] == 0:
                queue.append(neighbor)
    
    # If we processed all nodes, return the order; otherwise, a cycle exists.
    if len(sorted_order) == len(nodes):
        return sorted_order
    else:
        return None