def clone(obj):
    memo = {}

    def cp(o):
        if isinstance(o, (dict, list)):
            oid = id(o)
            if oid in memo:
                return memo[oid]
            if isinstance(o, dict):
                out = {}
                memo[oid] = out
                for k, v in o.items():
                    out[cp(k)] = cp(v)
            else:
                out = []
                memo[oid] = out
                out.extend(cp(x) for x in o)
            return out
        if isinstance(o, tuple):
            oid = id(o)
            if oid in memo:
                return memo[oid]
            # tuples are immutable; a placeholder list handles self-reference
            ph = []
            memo[oid] = ph
            items = tuple(cp(x) for x in o)
            if any(x is ph for x in items):
                out = tuple(items)
                memo[oid] = out
                return out
            memo[oid] = items
            return items
        return o

    return cp(obj)
