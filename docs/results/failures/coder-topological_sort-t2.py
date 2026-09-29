def topological_sort(deps):
    # Build reverse graph and calculate indegrees
    reverse_deps = {}
    all_nodes = set(deps.keys()) | {node for node_list in deps.values() for node in node_list}
    
    # Initialize reverse graph and indegrees
    indegrees = {node: 0 for node in all_nodes}
    
    for node, dependencies in deps.items():
        for dep in dependencies:
            if dep not in reverse_deps:
                reverse_deps[from_node] = []
            reverse_deps[dep].append(node)
            indegrees[node] += 1
    
    # Find nodes with zero indegree
    zero_indegree = [node for node in all_nodes if indegrees[node] == 0]
    result = []
    
    while zero_indegree:
        node = zero_indegree.pop()
        result.append(node)
        
        # Update indegrees of neighbors
        if node in reverse_deps:
            for neighbor in reverse_deps[node]:
                indegrees[neighbor] -= 1
                if indegrees[neighbor] == 0:
                    zero_indegree.append(neighbor)
    
    # Check if all nodes were processed
    if len(result) == len(all_nodes):
        return result
    else:
        return None