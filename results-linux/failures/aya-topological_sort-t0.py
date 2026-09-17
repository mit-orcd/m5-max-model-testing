def topological_sort(deps):
    # Create a set of all nodes
    all_nodes = set(deps.keys())
    # Create a set to keep track of visited nodes
    visited = set()
    # Create a list to store the topological order
    order = []

    def visit(node):
        if node in visited:
            return
        visited.add(node)
        for neighbour in deps.get(node, []):
            visit(neighbour)
        order.append(node)

    # Visit all nodes
    for node in all_nodes:
        visit(node)

    # If all nodes were visited, return the order
    if len(order) == len(all_nodes):
        return order
    else:
        return None