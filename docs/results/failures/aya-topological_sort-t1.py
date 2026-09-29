def topological_sort(deps):
    # Create a set of all nodes
    all_nodes = set(deps.keys())
    # Create a set to keep track of visited nodes
    visited = set()
    # Create a queue to store nodes to be processed
    q = []

    # Add all nodes to the queue
    for node in all_nodes:
        if node not in visited:
            q.append(node)

    # Process nodes in the queue
    while q:
        node = q.pop(0)
        visited.add(node)
        # Process dependencies of the current node
        for dep in deps.get(node, []):
            # If the dependency has not been visited yet, add it to the queue
            if dep not in visited:
                q.append(dep)

    # If all nodes have been visited, return the topological order
    if len(visited) == len(all_nodes):
        return list(visited)
    else:
        return None