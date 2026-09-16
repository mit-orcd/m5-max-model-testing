def topological_sort(deps):
    visited = {}
    order = []

    def visit(node):
        if node in visited:
            return not visited[node]  # True if currently visiting (cycle), False if done
        visited[node] = False
        for dep in deps.get(node, []):
            if not visit(dep):
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