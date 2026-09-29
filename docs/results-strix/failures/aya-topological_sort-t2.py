def topological_sort(deps):
    nodes = list(deps.keys())
    visited = set()
    order = []

    def visit(node):
        if node in visited:
            return
        visited.add(node)
        for neighbour in deps.get(node, []):
            visit(neighbour)
        order.append(node)

    for node in nodes:
        visit(node)
    return order[::-1]