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
    
    # Collect all nodes
    all_nodes = set(deps.keys())
    for dependents in deps.values():
        all_nodes.update(dependents)
    
    # Initialize in-degree and adjacency list
    for node in all_nodes:
        in_degree[node] = 0
        graph[node] = []
    
    # Populate in-degree and adjacency list
    for node, dependents in deps.items():
        for dependent in dependents:
            graph[node].append(dependent)
            in_degree[dependent] += 1
    
    # Initialize queue with nodes having no dependencies
    queue = [node for node in all_nodes if in_degree[node] == 0]
    
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
    
    # Check for cycle
    if len(result) != len(all_nodes):
        return None
    
    return result