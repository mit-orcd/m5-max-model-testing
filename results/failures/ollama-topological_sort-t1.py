```python
def topological_sort(deps):
    nodes = set(deps.keys())
    for node in deps:
        for dep in deps[node]:
            nodes.add(dep)

    WHITE, GRAY, BLACK = 0, 1, 2
    color = {node: WHITE for node in nodes}
    result = []

    def dfs(node):
        color[node] = GRAY
        for dep in deps.get(node,