def topological_sort(deps):
    visited = set()
    visiting = set()
    order = []

    def visit(node):
        if node in visiting:
            return False
        if node in visited:
            return True
        visiting.add(node)
        for dep in deps.get(node, []):
            if not visit(dep):
                return False
        visiting.remove(node)
        visited.add(node)
        order.append(node)
        return True

    for node in deps:
        if node not in visited:
            if not visit(node):
                return None
    return order[::-1]