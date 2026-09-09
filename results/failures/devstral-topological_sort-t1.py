def topological_sort(deps):
    in_degree = {}
    nodes = set()

    # Initialize in_degree and collect all nodes
    for node in deps:
        nodes.add(node)
        for dep in deps[node]:
            nodes.add(dep)
            in_degree[dep] = in_degree.get(dep, 0) + 1
        in_degree[node] = in_degree.get(node, 0)

    # Initialize queue with nodes having 0 in-degree
    queue = [node for node in nodes if in_degree.get(node, 0) == 0]
    if not queue and nodes:
        return None

    result = []
    while queue:
        node = queue.pop(0)
        result.append(node)

        for neighbor in deps.get(node, []):
            in_degree[neighbor] -= 1
            if in_degree[neighbor] == 0:
                queue.append(neighbor)

    return result if len(result) == len(nodes) else None