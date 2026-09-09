def topological_sort(deps):
    # Create a dictionary to store the in-degree (number of incoming edges) of each node
    in_degree = {node: 0 for node in deps}
    # Increment the in-degree count for each neighbor of a node
    for neighbors in deps.values():
        for neighbor in neighbors:
            in_degree[neighbor] += 1
    # Initialize a queue to store the nodes with an in-degree of 0 (no incoming edges)
    queue = [node for node, degree in in_degree.items() if degree == 0]
    # Initialize a list to store the sorted nodes
    sorted_nodes = []

    while queue:
        # Pop a node with no incoming edges from the queue
        node = queue.pop(0)
        sorted_nodes.append(node)
        # Decrease the in-degree count for each neighbor of the popped node
        for neighbor in deps[node]:
            in_degree[neighbor] -= 1
            # If a neighbor's in-degree becomes zero, add it to the queue
            if in_degree[neighbor] == 0:
                queue.append(neighbor)

    # If there are still nodes with incoming edges, return None (indicating a cycle)
    if any(in_degree.values()):
        return None
    # Otherwise, return the sorted list of nodes
    return sorted_nodes