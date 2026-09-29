def dedupe(items):
    seen = set()
    out = []
    append = out.append
    for x in items:
        if x not in seen:
            seen.add(x)
            append(x)
    return out
