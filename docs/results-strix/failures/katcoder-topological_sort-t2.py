def topological_sort(deps):
    all_nodes = set(deps.keys())
    for neighbors in deps.values():
        all_nodes.update(neighbors)
    
    for node in all_nodes:
        if node not in deps:
            deps[node] = []
    
    WHITE, GRAY, BLACK = 0, 1, 2
    color = {node: WHITE for node in all_nodes}
    result = []
    
    def dfs(node):
        color[node] = GRAY
        for dep in deps[node]:
            if color[dep] == GRAY:
                return None
            if color[dep] == WHITE:
                result = dfs(dep)
                if result is None:
                    return None
        color[node] = BLACK
        result.append(node)
        return result
    
    for node in all_nodes:
        if color[node] == WHITE:
            result = dfs(node)
            if result is None:
                return None
    
    return result[::-1]