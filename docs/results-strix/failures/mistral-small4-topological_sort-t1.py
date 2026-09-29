def topological_sort(deps):
    visited = set()
    visiting = set()
    sorted_nodes = []

    def visit(node):
        if node in visiting:
            return True
        if node in visited:
            return False
        visiting.add(node)
        for dependency in deps.get(node, []):
            if visit(dependency):
                return True
        visiting.remove(node)
        visited.add(node)
        sorted_nodes.insert(0, node)
        return False

    for node in deps:
        if node not in visited:
            if visit(node):
                return None
    return sorted_nodes