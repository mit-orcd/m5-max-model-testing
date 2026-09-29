def topological_sort(deps):
    # Build reverse dependency graph and compute in-degrees
    graph = {}
    in_degree = {}
    
    # Initialize all nodes
    all_nodes = set(deps.keys()) | set(node for node_list in deps.values() for node in node_list)
    
    # Build graph and compute in-degrees
    for node in all_nodes:
        if node not in graph:
            graph[node] = []
        if node not in in_degree:
            in_degree[node] = 0
    
    # Build the graph and update in-degrees
    for node, dependencies in deps.items():
        for dep in dependencies:
            if dep == node:
                return None  # Self-dependency
            if dep not in graph:
                graph[# Add dependency to graph
            graph[dep].append(node)
            in_degree[node] += 1

    # Topological sort using Kahn's algorithm
    queue = [node for node in all_nodes if in_degree[node] == 0]
    result = []
    
    while queue:
        node = queue.pop(0)
        result.append(node)
        
        for neighbor in graph.get(node, []):
            in_degree[neighbor] -= 1
            if in_degree[neighbor] == 0:
                queue.append(neighbor)
    
    # Check for cycles
    if len(result) != len(all_nodes):
        return None  # Cycle detected
    
    return result