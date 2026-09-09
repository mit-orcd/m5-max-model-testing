def topological_sort(deps):
    all_nodes = set(deps.keys())
    for dependencies in deps.values():
        all_nodes.update(dependencies)
    all_nodes = list(all_nodes)

    in_degree = {node: 0 for node in all_nodes}
    for node in deps:
        for dep in deps[node]:
            in_degree[dep] = in_degree.get(dep, 0) + 1

    queue = [node for node in all_nodes if in_degree[node] == 0]
    result = []

    while queue:
        node = queue.pop(0)
        result.append(node)
        for neighbor in all_nodes:
            if neighbor in deps and node in deps[neighbor]:
                in_degree[neighbor] -= 1
                if in_degree[neighbor] == 0:
                    queue.append(neighbor)

    return result if len(result) == len(all_nodes) else None