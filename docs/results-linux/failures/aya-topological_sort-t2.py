def topological_sort(deps):
    # Create a set of all nodes mentioned in the graph
    nodes = set(deps.keys()) | set(deps.values())

    # Create a queue of nodes with no dependencies
    q = [node for node in nodes if node not in deps]

    # Create a set to keep track of visited nodes
    visited = set()

    # Initialize the result list
    result = []

    while q:
        node = q.pop(0)
        if node in visited:
            return None  # Cycle detected
        visited.add(node)
        result.append(node)
        for neighbour in deps.get(node, []):
            nodes.remove(neighbour)
            if neighbour not in visited:
                q.append(neighbour)

    # Add remaining nodes (no dependencies) to the result
    result.extend(nodes)

    return result