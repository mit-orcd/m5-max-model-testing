def topological_sort(deps):
    all_nodes = set(deps.keys())
    for neighbors in deps.values():
        all_nodes.update(neighbors)
    
    for node in all_nodes:
        if node not in deps:
            deps = dict(deps)
            deps[node] = []
    
    WHITE, GRAY, BLACK = 0, 1, 2
    color = {node: WHITE for node in all_nodes}
    result = []
    
    def dfs(node):
        color[node] = GRAY
        for neighbor in deps[node]:
            if color[neighbor] == GRAY:
                return False
            if color[neighbor] == WHITE and not dfs(neighbor):
                return False
        color[node] = BLACK
        result.append(node)
        return True
    
    for node in all_nodes:
        if color[node] == WHITE:
            if not dfs(node):
                return None
    
    return result[::-1]