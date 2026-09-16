def topological_sort(deps):
    nodes = set(deps.keys()).union(*deps.values())
    in_degree = {node: 0 for node in nodes}
    for deps_list in deps.values():
        for dep in deps_list:
            in_degree[dep] += 1
    queue = [node for node in nodes if in_degree[node] == 0]
    sorted_list = []
    while queue:
        current = queue.pop(0)
        sorted_list.append(current)
        for node, dependencies in deps.items():
            if current in dependencies:
                in_degree[node] -= 1
                if in_degree[node] == 0:
                    queue.append(node)
    return sorted_list if len(sorted_list) == len(nodes) else None