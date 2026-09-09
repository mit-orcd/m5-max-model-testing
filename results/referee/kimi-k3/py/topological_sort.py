def topological_sort(deps):
    nodes = set(deps)
    for ds in deps.values():
        nodes.update(ds)
    remaining = {n: set(deps.get(n, [])) for n in nodes}
    order = []
    ready = [n for n in nodes if not remaining[n]]
    while ready:
        n = ready.pop()
        order.append(n)
        for m in nodes:
            if n in remaining[m]:
                remaining[m].discard(n)
                if not remaining[m]:
                    ready.append(m)
    return order if len(order) == len(nodes) else None
