```python
from collections import deque

def topological_sort(deps):
    nodes = list(deps.keys())
    node_set = set(nodes)

    for dependencies in deps.values():
        for dep in dependencies:
            if dep not in node_set:
                node_set.add(dep)
                nodes.append(dep)

    indegree = {node: len(set(deps.get(node, []))) for node in nodes}
    adjacency = {node: [] for node in nodes}

    for node, dependencies in deps.items():
        for dep in dict.fromkeys(dependencies):
            adjacency[dep].append(node)

    queue