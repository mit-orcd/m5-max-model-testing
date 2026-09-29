def topological_sort(deps):
    visited = set()
    visiting = set()
    sorted_nodes = []

    def visit(node):
        if node in visiting:
            return False
        if node in visited:
            return True
        visiting.add(node)
        for dependency in deps.get(node, []):
            if not visit(dependency):
                return False
        visiting.remove(node)
        visited.add(node)
        sorted_nodes.append(node)
        return True

    for node in deps:
        if not visit(node):
            return None
    for node in deps.values():
        for d in node:
            if d not in deps:
                if not visit(d):
                    return None
    return sorted_nodes[::-1]