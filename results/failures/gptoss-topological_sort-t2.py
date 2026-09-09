def topological_sort(deps):
    """Return a topological ordering of nodes in `deps`, or None if a cycle exists."""
    # Collect all nodes (keys and dependencies)
    all_nodes = set(deps.keys())
    for deps_list in deps.values():
        all_nodes.update(deps_list)

    # Build adjacency list: node -> list of dependencies
    adj = {node: deps.get(node, []) for node in all_nodes}

    # Visitation states: 0 = unvisited, 1 = visiting, 2 = visited
    state = {node: 0 for node in all_nodes}
    order = []

    def dfs(node):
        if state[node] == 1:          # cycle detected
            return False
        if state[node] == 2:          # already processed
            return True
        state[node] = 1
        for dep in adj.get(node, []):
            if not dfs(dep):
                return False
        state[node] = 2
        order.append(node)
        return True

    for node in all_nodes:
        if state[node] == 0:
            if not dfs(node):
                return None

    return order[::-1]  # reverse to get dependencies before dependents