def topological_sort(deps):
    # Create a dictionary to keep track of in-degree (number of incoming edges) of each node
    in_degree = {node: 0 for node in deps}
    for node_list in deps.values():
        for node in node_list:
            in_degree[node] += 1

    # Create a queue to store nodes with in-degree 0 (no incoming edges)
    queue = [node for node, degree in in_degree.items() if degree == 0]

    # Create a list to store the topological ordering of the nodes
    ordering = []

    # Iterate through the queue and process nodes
    while queue:
        node = queue.pop(0)
        ordering.append(node)
        for neighbour in deps[node]:
            in_degree[neighbour] -= 1
            if in_degree[neighbour] == 0:
                queue.append(neighbour)

    # If there are still nodes left in the in-degree dictionary, there is a cycle in the graph
    if len(in_degree) > 0:
        return None

    # Return the topological ordering of the nodes
    return ordering