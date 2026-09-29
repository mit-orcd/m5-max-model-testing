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

    for node in list(deps.keys()) + [d for deps_list in deps.values() for d in deps_list]:
        if node not in visited:
            if not visit(node):
                return None

    return sorted_nodes[::-1]