def clone(obj):
    def _clone(obj, seen):
        if isinstance(obj, (dict, list, tuple)):
            if obj in seen:
                return seen[obj]
            if isinstance(obj, dict):
                clone_obj = {}
                seen[obj] = clone_obj
                for k, v in obj.items():
                    clone_obj[_clone(k, seen)] = _clone(v, seen)
                return clone_obj
            elif isinstance(obj, list):
                clone_obj = []
                seen[obj] = clone_obj
                for item in obj:
                    clone_obj.append(_clone(item, seen))
                return clone_obj
            else:  # tuple
                clone_obj = ()
                seen[obj] = clone_obj
                for item in obj:
                    clone_obj = clone_obj + (_clone(item, seen),)
                return clone_obj
        else:
            return obj
    return _clone(obj, {})