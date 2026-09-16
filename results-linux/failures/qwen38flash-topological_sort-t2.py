from collections import deque

def topological_sort(deps):
    """
    deps: dict mapping node -> list of dependencies (nodes it depends on)
    Returns: list of nodes in topological order, or None if cycle detected
    """
    # Collect all nodes: those in keys and those mentioned as dependencies
    all_nodes = set(deps.keys())
    for node, dependency_list in deps.items():
        for dep in dependency_list:
            all_nodes.add(dep)
    
    # Build adjacency list and in-degree count
    # Edge: dep -> node (because node depends on dep, so dep must come before node)
    adj = {node: [] for node in all_nodes}
    in_degree = {node: 0 for node in all_nodes}
    
    for node, dependency_list in deps.items():
        for dep in dependency_list:
            adj[dep].append(node)
            in_degree[node] += 1
    
    # Initialize queue with nodes having in_degree 0
    queue = deque()
    for node in all_nodes:
        if in_degree[node] == 0:
            queue.append(node)
    
    result = []
    while queue:
        # Process nodes in a deterministic order (sorted for consistency)
        # To ensure deterministic output, we can sort the queue items
        # But BFS with queue doesn't guarantee order among same-level nodes
        # Let's use a sorted approach: pick smallest available node each time
        # Actually, let's just use a regular queue but sort the initial nodes
        # For deterministic output, we should process nodes in sorted order when multiple are available
        
        # Let me redo this with a priority queue approach or just sort at each step
        # Simpler: use a list and sort, picking from front
        pass
    
    # Redo with a more careful approach for deterministic output
    queue = sorted([n for n in all_nodes if in_degree[n] == 0])
    queue = deque(queue)
    
    result = []
    while queue:
        node = queue.popleft()
        result.append(node)
        for neighbor in sorted(adj[node]):
            in_degree[neighbor] -= 1
            if in_degree[neighbor] == 0:
                queue.append(neighbor)
    
    if len(result) != len(all_nodes):
        return None
    
    return result