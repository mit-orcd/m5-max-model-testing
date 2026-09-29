def topological_sort(deps):
    """
    Performs a topological sort on a directed graph.
    :param deps: A dictionary mapping a node to a list of its dependencies.
    :return: A list of nodes in topological order, or None if a cycle is detected.
    """
    # Initialize state: 0 = unvisited, 1 = visiting, 2 = visited
    state = {node: 0 for node in deps}
    # Add nodes that are only dependencies to the state dict
    for dependencies in deps.values():
        for dep in dependencies:
            if dep not in state:
                state[dep] = 0

    result = []

    def visit(node):
        if state[node] == 1:
            # We found a back edge, indicating a cycle
            return False
        if state[node] == 2:
            # Already processed, no need to re-process
            return True

        # Mark as visiting
        state[node] = 1

        # Recursively visit all dependencies
        for dep in deps.get(node, []):
            if not visit(dep):
                return False

        # Mark as visited and add to result
        state[node] = 2
        result.append(node)
        return True

    # Iterate over all nodes to handle disconnected components
    for node in state:
        if state[node] == 0:
            if not visit(node):
                return None

    # The result list is built in reverse post-order, so reverse it for the final output
    return result[::-1]