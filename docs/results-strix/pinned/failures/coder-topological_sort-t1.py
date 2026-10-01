def topological_sort(deps):
    # Build reverse dependency map and in-degree count
    reverse_deps = {node: [] for node in deps}
    in_degree = {node: 0 for node in deps}
    
    # Initialize in-degree for all nodes
    for node, dependencies in deps.items():
        for dep in dependencies:
            if dep not in reverse_deps:
                reverse_deps[dep] = []
            reverse_deps[dep].append(node)
            in_degree[dep] = in_degree.get(dep, 0) + 1
    
    # Add nodes that are only dependencies (no outgoing edges)
    all_nodes = set(in_degree.keys())
    for node in reverse_deps:
        if node not in all_nodes:
            in_degree[node] = 0
    
    # Find all nodes with no incoming edges
    queue = [node for node, degree in in_degree.items() if degree == 0]
    result = []
    
    while queue:
        node = queue.pop(0)
        result.append(node)
        
        # Remove node and update in-degrees
        for neighbor in reverse_deps.get(node, []):
            in_degree[neighbor] -= 1
            if in_degree[neighbor] == 0:
                queue.append(neighbor)
    
    # Check for cycles
    if len(result) != len(in_degree):
        return None
    
    return result