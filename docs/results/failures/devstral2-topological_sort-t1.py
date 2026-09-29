def topological_sort(deps):
    in_degree = {}
    nodes = set(deps.keys())

    # Collect all nodes, including those only mentioned as dependencies
    for dependencies in deps.values():
        for dep in dependencies:
            nodes.add(dep)

    # Initialize in_degree for all nodes
    for node in nodes:
        in_degree[node] = 0

    # Calculate in_degree for each node
    for node, dependencies in deps.items():
        for dep in dependencies:
            if dep in in_degree:
                in_degree[dep] += 1
            else:
                in_degree[dep] = 1

    # Queue for nodes with in_degree 0
    queue = []
    for node in in_degree:
        if in_degree[node] == 0:
            queue.append(node)

    topo_order = []
    while queue:
        node = queue.pop(0)
        topo_order.append(node)

        for neighbor in deps.get(node, []):
            in_degree[neighbor] -= 1
            if in_degree[neighbor] == 0:
                queue.append(neighbor)

    if len(topo_order) == len(nodes):
        return topo_order
    else:
        return None