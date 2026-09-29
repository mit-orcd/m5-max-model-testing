def topological_sort(deps):
    visited = {}
    order = []

    def visit(node):
        if node in visited:
            return not visited[node]
        visited[node] = False
        for neighbor in deps.get(node, []):
            if not visit(neighbor):
                return False
        visited[node] = True
        order.append(node)
        return True

    all_nodes = set(deps.keys())
    for node in deps.values():
        all_nodes.update(node)
    for node in all_nodes:
        if node not in visited:
            if not visit(node):
                return None
    return order[::-1]