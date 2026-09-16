def topological_sort(deps):
    # Create a dictionary to keep track of in-degree (number of incoming edges)
    # of each node.
    in_degree = {node: 0 for node in deps}
    for node_deps in deps.values():
        for node in node_deps:
            in_degree[node] += 1

    # Create a queue to store nodes with in-degree 0 (no incoming edges).
    q = [node for node, degree in in_degree.items() if degree == 0]

    # Create a list to store the topological order of the nodes.
    order = []

    # Iterate through the queue and process each node.
    while q:
        node = q.pop(0)
        order.append(node)

        # Decrease the in-degree of nodes that this node depends on.
        for dep in deps[node]:
            in_degree[dep] -= 1

            # If a node's in-degree becomes zero, add it to the queue.
            if in_degree[dep] == 0:
                q.append(dep)

    # If there are still nodes with incoming edges, return None
    # as the graph has a cycle.
    if any(in_degree.values()):
        return None

    return order