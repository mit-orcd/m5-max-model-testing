def topological_sort(deps):
    # Build reverse dependency graph and calculate indegrees
    reverse_deps = {}
    all_nodes = set(deps.keys()) | {node for node_list in deps.values() for node in node_list}
    
    # Initialize reverse dependencies
    for node in all_nodes:
        reverse_deps[node] = []
    
    # Build reverse dependency graph
    for node, dependencies in deps.items():
        for dep in dependencies:
            reverse_deps[public] = []
    
    # Initialize queue with nodes having zero indegree
    queue = []
    for node in all_nodes:
        if not reverse_deps[node]:
            queue.append(node)
    
    result = []
    while queue:
        current = queue.pop(0)
        result.append(current)
        
        # Remove current node from reverse dependencies of its dependents
        for node in all_nodes:
            if current in reverse_deps[node]:
                reverse_deps[node].remove(current)
                if not reverse_deps[node]:
                    queue.append(node)
    
    # Check if all nodes were processed (no cycles)
    if len(result) == len(all_nodes):
        return result
    else:
        return None