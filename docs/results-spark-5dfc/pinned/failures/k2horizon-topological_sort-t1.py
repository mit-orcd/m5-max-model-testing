def topological_sort(deps):
    # Build full graph including nodes that appear only as dependencies
    graph = {node: set(d) for node, d in deps.items()}
    for node, dlist in deps.items():
        for d in dlist:
            if d not in graph:
                graph[d] = set()
    
    # Kahn's algorithm
    indegree = {node: 0 for node in graph}
    for node, dlist in graph.items():
        for d in dlist:
            indegree[node] += 1
    
    queue = [node for node, deg in indegree.items() if deg == 0]
    result = []
    
    while queue:
        node = queue.pop(0)
        result.append(node)
        for neighbor in graph[node]:
            indegree[neighbor] -= 1
            if indegree[neighbor] == 0:
                queue.append(neighbor)
    
    if len(result) != len(graph):
        return None
    
    return result