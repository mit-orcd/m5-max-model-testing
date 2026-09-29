def topological_sort(deps):
    graph = {}
    for node, dependencies in deps.items():
        graph.setdefault(node, [])
        for dep in dependencies:
            graph.setdefault(dep, [])
            graph[node].append(dep)

    state = {}
    order = []

    def visit(node):
        if state.get(node) == 1:
            return
        if state.get(node) == 2:
            return
        state[node] = 1
        for dep in graph[node]:
            visit(dep)
        state[node] = 2
        order.append(node)

    for node in graph:
        if node not in state:
            visit(node)

    if len(order) != len(graph):
        return None
    order.reverse()
    return order