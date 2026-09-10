def topological_sort(deps):
    # Build full graph including nodes that appear only as dependencies
    graph = {}
    for node, dependencies in deps.items():
        graph.setdefault(node, [])
        for dep in dependencies:
            graph.setdefault(dep, [])
            graph[node].append(dep)

    # Kahn's algorithm
    in_degree = {node: 0 for node in graph}
    for node, dependencies in graph.items():
        for dep in dependencies:
            in_degree[dep] += 1

    queue = [node for node in graph if in_degree[node] == 0]
    result = []

    while queue:
        node = queue.pop(0)
        result.append(node)
        for dep in graph[node]:
            in_degree[dep] -= 1
            if in_degree[dep] == 0:
                queue.append(dep)

    if len(result) != len(graph):
        return None
    return result