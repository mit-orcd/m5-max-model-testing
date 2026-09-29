from collections import deque

def topological_sort(deps):
    nodes = set(deps)
    graph = {node: set() for node in nodes}
    indegree = {node: 0 for node in nodes}

    for node, dependencies in deps.items():
        for dep in set(dependencies):
            nodes.add(dep)
            graph.setdefault(dep, set()).add(node)
            indegree[node] += 1

    queue = deque(node for node in nodes if indegree[node] == 0)
    result = []

    while queue:
        node = queue.popleft()
        result.append(node)

        for dependent in graph[node]:
            indegree[dependent] -= 1
            if indegree[dependent] == 0:
                queue.append(dependent)

    if len(result) != len(nodes):
        return None

    return result