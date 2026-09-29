def topological_sort(deps):
    all_nodes = set(deps.keys())
    for neighbors in deps.values():
        for n in neighbors:
            all_nodes.add(n)
    
    WHITE, GRAY, BLACK = 0, 1, 2
    color = {n: WHITE for n in all_nodes}
    result = []
    
    def dfs(node):
        color[node] = GRAY
        for dep in deps.get(node, []):
            if color[dep] == GRAY:
                return False
            if color[dep] == WHITE:
                if not dfs(dep):
                    return False
        color[node] = BLACK
        result.append(node)
        return True
    
    for node in all_nodes:
        if color[node] == WHITE:
            if not dfs(node):
                return None
    
    return result[::-1]