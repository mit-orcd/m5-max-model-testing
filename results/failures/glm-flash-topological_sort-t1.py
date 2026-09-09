def topological_sort(deps):
    # Initialize the set of all nodes (including those with no dependencies)
    nodes = set()
    for dependencies in deps.values():
        nodes.update(dependencies)
    nodes.update(deps.keys())
    nodes = list(nodes)

    # Kahn's algorithm using a set for O(1) membership checks
    in_degree = {node: 0 for node in nodes}

    for dependencies in deps.values():
        for dependency in dependencies:
            if dependency in in_degree:  # Only track if dependency is a node in our graph
                in_degree[dependency] += 1

    # Initialize queue with nodes that have no dependencies
    queue = [node for node in nodes if in_degree[node] == 0    ]

    topo_order =    []

    while queue:
        # We use a list as a queue (FIFO)
        current = queue.pop(  0)
        topo_order.append(current)

        if current in deps:
            for neighbor in deps[current]:
                if in_degree[neighbor] >  0:
                    in_degree[neighbor] -=  1
                    if in_degree[neighbor] ==  0:
                        queue.append(neighbor)

    if len(topo_order) == len(nodes):
        return topo_order
    else:
        return None