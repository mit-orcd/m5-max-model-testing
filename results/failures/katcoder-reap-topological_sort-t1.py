def topological_sort(deps):
    all_nodes = set(deps.keys())
    for children in deps.values():
        all_nodes.update(children)

    for node in all_nodes:
        if node not in deps:
            deps[node] = []

    WHITE, GRAY, BLACK = 0, 1, 2
    color = {node: WHITE for node in all_nodes}
    result = []

    def dfs(u):
        color[u] = GRAY
        for v in deps[u]:
            if color[v] == GRAY:
                return False
            if color[v] == WHITE:
                if not dfs(v):
                    return False
        color[u] = BLACK
        result.append(u)
        return True

    for node in all_nodes:
        if color[node] == WHITE:
            if not dfs(node):
                return None

    return result[::-1]