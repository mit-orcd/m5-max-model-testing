def topological_sort(deps):
    """
    Performs a topological sort on a directed graph defined by dependencies.
    
    Args:
        deps: A dictionary mapping a node (str) to a list of nodes it depends on.
    
    Returns:
        A list of nodes in topological order, or None if a cycle is detected.
    """
    # Initialize in-degree count for all nodes
    in_degree = {}
    # Initialize adjacency list
    graph = {}
    
    # Populate in-degree and adjacency list
    # We iterate through all nodes to ensure isolated nodes are included
    for node in deps:
        graph[node] = deps[node]
        in_degree[node] = 0
        
        for dep in deps[node]:
            if dep not in in_degree:
                in_degree[dep] = 0
            in_degree[node] += 1
            
    # Add nodes that are only dependencies (not keys in deps)
    for node in in_degree:
        if node not in graph:
            graph[node] = []
            
    # Initialize queue with nodes having zero in-degree
    queue = [node for node in in_degree if in_degree[node] == 0]
    
    result = []
    
    while queue:
        # Sort queue to ensure deterministic output (optional but good for testing)
        queue.sort()
        current = queue.pop(0)
        result.append(current)
        
        for neighbor in graph[current]:
            in_degree[neighbor] -= 1
            if in_degree[neighbor] == 0:
                queue.append(neighbor)
                
    # Check for cycle: if result length is less than total nodes, there is a cycle
    if len(result) != len(in_degree):
        return None
        
    return result