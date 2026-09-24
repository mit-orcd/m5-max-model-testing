```python
def clone(obj):
    """
    Deep-copies a structure of dicts, lists, tuples and scalars without using the copy module.
    Handles cycles and shared references correctly.
    """
    # We need to track visited objects to handle cycles and shared references.
    # Key: id(original_object) -> Value: cloned_object
    seen = {}

    def _clone(o):
        # Check if we've already cloned this object
        oid = id(o)
        if oid in seen:
            return seen[oid]

        # Scalars (int, float, str, bool, None, etc.) are immutable
        # We can return them directly
        if o is None:
            return None
        if isinstance(o, bool):
            return o
        if isinstance(o, int):
            return o
        if isinstance(o, float):
            return o
        if isinstance(o, str):
            return o
        if isinstance(o, tuple):
            # Tuples are immutable, but they can contain mutable elements.
            # We need to deep-copy the elements.
            # However, if a tuple contains itself or has shared references,
            # we need to handle it carefully.
            # Since tuples are immutable, we cannot create an empty tuple and fill it later.
            # We must create the tuple after cloning its elements.
            # But wait: if a tuple contains itself, we have a cycle.
            # A tuple containing itself is possible: t = ([],); t[0].append(t)
            # In this case, t is a tuple, t[0] is a list, t[0][0] is t.
            # When we clone t, we need to clone t[0].
            # When we clone t[0] (the list), we need to clone t[0][0] which is t.
            # But t is currently being cloned.
            # This creates a problem for immutable containers like tuples.
            # Actually, in Python, tuples are immutable, so a tuple cannot directly contain itself.
            # But it can contain a list that contains the tuple.
            # Let's think about how to handle this.
            # If we encounter a tuple, we need to clone its elements.
            # If one of the elements refers back to this tuple (indirectly via a mutable object),
            # we might have a problem.
            # Example: l = []; t = (l,); l.append(t)
            # id(t) is the tuple. id(l) is the list.
            # When cloning t:
            #   We see t. We haven't seen it before.
            #   We need to clone its elements: l.
            #   We call _clone(l).
            #   We see l. We haven't seen it before.
            #   We create a new list new_l. We mark seen[id(l)] = new_l.
            #   We need to clone elements of l. l contains t.
            #   We call _clone(t).
            #   We see t. We haven't seen it before in `seen`?
            #   Wait, did we mark t as seen before starting to clone its elements?
            #   If we don't mark t as seen before cloning its elements, we'll recurse infinitely.
            #   If we do mark t as seen, we need to have a placeholder for the cloned tuple.
            #   But we can't create a placeholder for a tuple because it's immutable.
            #   This is a known issue. How do standard deep copy implementations handle this?
            #   The `copy` module uses `__deepcopy__` or `reconstruct` for tuples.
            #   For tuples, `copy.deepcopy` clones the elements first, then creates the tuple.
            #   If there's a cycle involving a tuple, it might fail or behave unexpectedly?
            #   Let's check: Python's copy.deepcopy does handle this.
            #   How?
            #   Actually, for immutable containers, if there's a cycle, it's tricky.
            #   Let's re-read the problem. "Cycles must not cause infinite recursion: if obj contains itself, so must the clone."
            #   If a tuple contains itself directly, it's impossible.
            #   If a tuple contains a list that contains the tuple, is that "containing itself"?
            #   The structure contains itself indirectly.
            #   Let's assume the standard approach:
            #   For mutable containers (list, dict), we can create an empty one, register it, then fill it.
            #   For tuples, we can't.
            #   However, if a tuple is part of a cycle, it must be reachable from a mutable object.
            #   Let's try to implement it and see.
            
            # Create a placeholder? No, we can't.
            # Maybe we clone the elements first, and if a cycle is detected among elements,
            # we handle it then.
            
            # Let's look at how `copy.deepcopy` handles tuples.
            # It calls `cls.__new__(cls)` or similar, then fills it.
            # For tuples, it creates the tuple after cloning elements.
            # If cloning an element leads back to the tuple, `deepcopy` will find the tuple in the memo.
            # But the tuple isn't in the memo yet because it hasn't been created.
            # Actually, `copy.deepcopy` uses `id(obj)` as key.
            # Let's test:
            # l = []
            # t = (l,)
            # l.append(t)
            # import copy
            # c = copy.deepcopy(t)
            # This works. How?
            # It seems `copy.deepcopy` might have special handling or relies on the fact that
            # when it encounters `t` again via `l`, it has already started processing `t`?
            # No, if `t` is not in memo, it will try to deepcopy `t` again.
            # Let's check the source or behavior.
            # Actually, if you do:
            # l = []
            # t = (l,)
            # l.append(t)
            # copy.deepcopy(t)
            # It works.
            # It seems that when it encounters `t` inside `l`, it finds `t` in the memo.
            # This implies `t` was added to the memo before its elements were cloned.
            # But how can you add an immutable object to the memo before it's fully constructed?
            # You can't.
            
            # Wait, maybe the cycle is broken because `t` is not mutated.
            # If `t` is not in memo, and we try to clone it again, we get infinite recursion.
            # Let's trace:
            # deepcopy(t):
            #   id(t) not in memo.
            #   Create new_t = deepcopy(tuple, memo)? No.
            #   It clones elements: deepcopy(l, memo).
            #   deepcopy(l, memo):
            #     id(l) not in memo.
            #     new_l = [].
            #     memo[id(l)] = new_l.
            #     For item in l:
            #       deepcopy(item, memo) where item is t.
            #       deepcopy(t, memo):
            #         id(t) not in memo.
            #         ... recursion!
            
            # So how does `copy.deepcopy` work?
            # Maybe it doesn't add `t` to memo until it's done?
            # Then it would infinite recurse.
            # Let's actually run a quick mental check or recall.
            # Yes, `copy.deepcopy` handles this.
            # The key is that for tuples, it might use a different strategy.
            # Or maybe I'm wrong about the cycle.
            # If `t` contains `l`, and `l` contains `t`, then `t` is reachable from `l`.
            # When cloning `t`, we start cloning `l`.
            # When cloning `l`, we see `t`.
            # If `t` is not in memo, we recurse.
            
            # Let's check Python documentation or source.
            # From Python docs: "The copy module uses __deepcopy__ to control copying."
            # For built-in types, it's implemented in C.
            # In CPython, `deepcopy` for tuple:
            # It creates a new tuple by copying elements.
            # If a cycle exists, it relies on the memo.
            # But the memo is updated *after* the object is created?
            # If so, it would fail for this case.
            
            # Let's test this hypothesis.
            # Actually, I recall that for immutable objects, `copy.deepcopy` might not add them to the memo in the same way,
            # or it might add a placeholder.
            # But you can't have a placeholder for a tuple.
            
            # Alternative theory: The problem statement says "structure of dicts, lists, tuples and scalars".
            # It does not explicitly say that cycles involving tuples are supported in a way that requires breaking the immutability constraint during construction.
            # However, "Cycles must not cause infinite recursion".
            
            # Let's look at a solution that works for lists and dicts.
            # For lists: create empty list, add to memo, then fill.
            # For dicts: create empty dict, add to memo, then fill.
            # For tuples: this is the hard part.
            
            # If a tuple is involved in a cycle, it must be reachable from a mutable object.
            # Let's say we have `l = []; t = (l,); l.append(t)`.
            # When we clone `t`:
            # We need to clone `l`.
            # When we clone `l`, we need to clone `t`.
            # If we haven't stored `t` in memo, we recurse.
            
            # Solution: We can't store the final tuple in memo before it's created.
            # But we can store a "proxy" or handle it differently.
            # Or, we can note that if we encounter the same tuple ID again, it means we are in a cycle.
            # But we don't have the clone yet.
            
            # Actually, there is a trick. We can use a two-pass approach or a specialized handler.
            # But the simplest way that works for `copy.deepcopy` is that it *does* work.
            # How?
            # CPython's `deepcopy` for tuple:
            # 1. Checks memo.
            # 2. If not in memo, it creates a new tuple by deep-copying each element.
            # 3. It stores the new tuple in memo *after* creation.
            # This would cause infinite recursion for the case above.
            
            # Wait, let me double check.
            # If I run:
            # l = []
            # t = (l,)
            # l.append(t)
            # import copy
            # copy.deepcopy(t)
            # Does it work?
            # Yes, it works.
            
            # So how does it avoid recursion?
            # Perhaps `t` is added to the memo before its elements are copied?
            # But how?
            
            # Ah, I might be misremembering. Let's look at the C code logic conceptually.
            # Maybe it adds a placeholder?
            
            # Let's try a different approach.
            # If we encounter a tuple, we clone its elements.
            # If cloning an element leads back to this tuple, we have a problem.
            # But notice: `t` is immutable.
            # If `l` contains `t`, then `t` is an element of `l`.
            # When cloning `l`, we clone `t`.
            # When cloning `t`, we clone `l`.
            
            # Maybe the key is that `id(t)` is checked.
            # If we are inside `_clone(t)`, and we call `_clone(l)`, and inside `_clone(l)` we call `_clone(t)`,
            # we see `id(t)` is being processed.
            
            # We can maintain a set of `ids` currently being processed (stack).
            # If we see an `id` that is currently being processed, we have a cycle.
            # But we don't have the clone yet.
            
            # For lists and dicts, we solve this by creating the empty container and registering it.
            # For tuples, we can't.
            
            # However, if a tuple is in a cycle, it implies the cycle goes through a mutable object.
            # Let's say we have `t` and `l`.
            # `_clone(t)` starts.
            # It needs to clone `l`.
            # `_clone(l)` starts.
            # It creates `new_l = []`. Registers `id(l) -> new_l`.
            # It iterates over `l`. First element is `t`.
            # It calls `_clone(t)`.
            # `_clone(t)` sees `id(t)`.
            # If we haven't registered `t`, we recurse.
            
            # What if we register `t` with a placeholder before cloning its elements?
            # We can't create a tuple placeholder.
            
            # Is it possible that the problem guarantees that cycles only involve mutable containers in a way that allows this?
            # "if obj contains itself".
            # Usually, this means `obj` is mutable.
            # If `obj` is a tuple, it cannot contain itself directly.
            # Indirectly, yes.
            
            # Let's assume the standard behavior of `copy.deepcopy` is the target.
            # If `copy.deepcopy` works, there is a way.
            
            # Actually, I found a reference:
            # The `copy` module's `deepcopy` function, for tuples, does not add the tuple to the memo until after it is copied.
            # This means the case `l = []; t = (l,); l.append(t)` *should* cause infinite recursion in a naive implementation.
            # But `copy.deepcopy` handles it.
            
            # Let's check the actual behavior.
            # If I run:
            # l = []
            # t = (l,)
            # l.append(t)
            # copy.deepcopy(t)
            # It returns a new tuple.
            
            # So how?
            # Maybe the cycle is detected because `l` is already in the memo when `t` is encountered?
            # No, `t` is encountered when cloning `l`.
            
            # Let's trace carefully.
            # `deepcopy(t, memo={})`
            # `id(t)` not in memo.
            # `y = deepcopy(tuple, memo)` ? No.
            # It calls `cls = type(x)`. `cls` is tuple.
            # It checks `issubclass(cls, tuple)`?
            # It creates a new tuple by copying elements.
            # `y = tuple(deepcopy(item, memo) for item in x)`
            # So it iterates items of `t`. Item is `l`.
            # `deepcopy(l, memo)`
            # `id(l)` not in memo.
            # `y = []`
            # `memo[id(l)] = y`
            # Iterate items of `l`. Item is `t`.
            # `deepcopy(t, memo)`
            # `id(t)` not in memo.
            # `y = tuple(deepcopy(item, memo) for item in t)`
            # Item is `l`.
            # `deepcopy(l, memo)`
            # `id(l)` IS in memo. Returns `memo[id(l)]` which is `new_l`.
            # So `deepcopy(t)` returns `tuple([new_l])`.
            # Then `memo[id(t)] = tuple([new_l])`.
            # Back to first `deepcopy(t)`.
            # `y = tuple([new_l])`.
            # `memo[id(t)] = y` (overwrite or check).
            
            # So it seems it *does* work without infinite recursion because `l` was registered in memo before `t` was fully processed?
            # Wait.
            # In the first `deepcopy(t)`, `id(t)` is NOT in memo.
            # It starts cloning elements.
            # It calls `deepcopy(l)`.
            # `deepcopy(l)` registers `id(l)`.
            # `deepcopy(l)` tries to clone `t`.
            # `deepcopy(t)` is called. `id(t)` is NOT in memo.
            # It starts cloning elements of `t`.
            # It calls `deepcopy(l)`.
            # `deepcopy(l)` finds `id(l)` in memo. Returns `new_l`.
            # So `deepcopy(t)` (the inner one) completes. It creates `new_t_inner = (new_l,)`.
            # It registers `id(t) -> new_t_inner`.
            # Returns `new_t_inner`.
            # Back to `deepcopy(l)`. It gets `new_t_inner`.
            `deepcopy(l)` finishes. Returns `new_l` (which contains `new_t_inner`).
            # Back to first `deepcopy(t)`.
            # It gets `new_l`.
            # It creates `new_t_outer = (new_l,)`.
            # It registers `id(t) -> new_t_outer`.
            # Returns `new_t_outer`.
            
            # So we have two different tuples being created for the same original `t`!
            # `new_t_inner` and `new_t_outer`.
            # `new_t_inner` is `(new_l,)`.
            # `new_t_outer`