```python
def topological_sort(deps):
    all_nodes = set(deps.keys())
    for node in deps:
        for dep in deps[node]:
            all_nodes.add(dep)

    WHITE, GRAY, BLACK = 0, 1, 2
    color = {node: WHITE for node in all_nodes}
    result = []

    def dfs(node):
        color[node] = GRAY
        for dep in deps.get(node, []):
            if color[dep] == GRAY:
                return False
            if