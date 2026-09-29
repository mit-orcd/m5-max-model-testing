```python
def clone(obj):
    """
    Deep-copies a structure of dicts, lists, tuples and scalars without using the copy module.
    Handles cycles and shared references correctly.
    """
    # We need to handle cycles and shared references.
    # We'll use a dictionary to map original object ids to their clones.
    # This ensures that if an object is encountered again, we return the already-created clone.
    
    def _clone(obj, memo):
        # Check if we've already cloned this object
        obj_id = id(obj)
        if obj_id in memo:
            return memo[obj_id]
        
        # Scalars (int, float, str, bool, None, etc.) are immutable, so we can return them directly.
        # However, to be safe and general, we check types.
        # The problem states: dicts, lists, tuples and scalars.
        # Tuples are immutable but can contain mutable objects, so they need deep copying of contents.
        # Dicts and lists are mutable and need deep copying.
        
        if isinstance(obj, dict):
            # Create a new dict and register it in memo BEFORE filling it, to handle cycles
            new_dict = {}
            memo[obj_id] = new_dict
            for key, value in obj.items():
                # Keys in dicts are typically immutable, but could be tuples containing mutable objects?
                # In Python, dict keys must be hashable. Tuples are hashable if their elements are.
                # If a tuple key contains a mutable object, it wouldn't be hashable.
                # So keys are effectively immutable scalars or tuples of scalars.
                # We should still deep copy keys if they are containers? 
                # Actually, if a key is a tuple, it's immutable, but its contents might be mutable?
                # No, if a tuple contains a mutable object, the tuple is not hashable.
                # So dict keys are always immutable and don't need deep copying in the sense of creating new containers
                # unless they are tuples that we want to clone? 
                # The problem says "shared references must stay shared". 
                # If a key is a tuple, and that same tuple object is used elsewhere, it should be cloned once.
                # But since tuples are immutable, cloning them is just returning the same object or a new one with same contents.
                # To be consistent with "no mutable object may be shared", and since tuples are immutable, 
                # we can treat them carefully.
                # Let's deep copy keys as well to be safe, using the same memo.
                new_key = _clone(key, memo)
                new_value = _clone(value, memo)
                new_dict[new_key] = new_value
            return new_dict
        
        elif isinstance(obj, list):
            # Create a new list and register it in memo BEFORE filling it, to handle cycles
            new_list = []
            memo[obj_id] = new_list
            for item in obj:
                new_list.append(_clone(item, memo))
            return new_list
        
        elif isinstance(obj, tuple):
            # Tuples are immutable. We need to create a new tuple with cloned elements.
            # We register the new tuple in memo before creating it? 
            # Since tuples are immutable, they can't contain themselves directly in a way that causes infinite recursion 
            # during construction, but they can contain objects that refer back.
            # However, we can't create a tuple with placeholders easily.
            # But since tuples are immutable, if we encounter the same tuple object again, we should return the same clone.
            # We can create the tuple after cloning all elements.
            # But wait: if a tuple contains a list that contains the tuple, we have a cycle.
            # Example: t = ([],); t[0].append(t)
            # When cloning t, we clone the list. The list contains t. 
            # When cloning the list's element t, we need to have already registered t's clone.
            # So we need to register the tuple's clone before cloning its elements? 
            # We can't create a tuple with unknown length or contents before cloning elements.
            # Alternative: For tuples, since they are immutable, we can first check if it's in memo.
            # If not, we need to create it. But to handle cycles, we need to register it before recursing into elements.
            # We can create a "placeholder" or just accept that for tuples, we might need a two-pass approach or 
            # rely on the fact that if a tuple is part of a cycle, the cycle must go through a mutable container.
            # Actually, if t is a tuple, and t contains a list L, and L contains t, then:
            # clone(t) -> clone(L) -> clone(t). 
            # When clone(t) is called the second time, t is not in memo yet because we haven't finished creating it.
            # This is a problem.
            # Solution: For tuples, we can't easily register before creation. 
            # But note: if a tuple is involved in a cycle, the cycle must pass through a mutable object (list or dict).
            # So when we encounter the tuple again via the mutable object, we haven't registered the tuple's clone yet.
            # This would lead to infinite recursion.
            # How to fix? We can't create a tuple with placeholders.
            # One approach: For tuples, we don't register them in memo until after they are created.
            # But then cycles involving tuples won't be handled correctly.
            # Wait: if t is a tuple, and t[0] is a list L, and L[0] is t.
            # clone(t): 
            #   t not in memo.
            #   clone elements: clone(L)
            #     L not in memo.
            #     new_L = []
            #     memo[id(L)] = new_L
            #     clone(L[0]) which is t
            #       t not in memo.
            #       clone elements: clone(L)
            #         L is in memo, return new_L
            #       So we try to create a new tuple with new_L as element.
            #       But this new tuple is not the same as the one we're currently creating for t.
            #       This leads to infinite recursion because we keep trying to create a new tuple for t.
            # 
            # Actually, the issue is that we need to register the clone of t before we finish cloning its elements.
            # Since tuples are immutable, we can't do that easily.
            # However, in practice, if a tuple is part of a cycle, the cycle must go through a mutable container.
            # The standard solution for this is to use a two-pass approach or to handle tuples specially.
            # 
            # Alternative idea: Since tuples are immutable, we can treat them as "atomic" for the purpose of memoization 
            # if they don't contain mutable objects. But if they do, we need to clone their contents.
            # 
            # Let's think differently. The problem says "cycles must not cause infinite recursion".
            # If obj contains itself, so must the clone.
            # Example: l = []; l.append(l). clone(l) should have clone_l[0] is clone_l.
            # Example: t = (None,); t = (t,) -> this is not possible because tuples are immutable.
            # You can't have a tuple that directly contains itself.
            # But you can have: l = []; t = (l,); l.append(t)
            # Then t contains l, and l contains t.
            # clone(t):
            #   t not in memo.
            #   clone elements: clone(l)
            #     l not in memo.
            #     new_l = []
            #     memo[id(l)] = new_l
            #     clone(l[0]) which is t
            #       t not in memo.
            #       clone elements: clone(l)
            #         l is in memo, return new_l
            #       So we create a new tuple with new_l as element. Let's call it new_t1.
            #       But we are still in the process of cloning t. We haven't registered t's clone yet.
            #       So new_t1 is created, but it's not registered as the clone of t.
            #       Then we return new_t1.
            #     So new_l.append(new_t1)
            #     Return new_l.
            #   So new_t = (new_l,)
            #   But new_l contains new_t1, which is (new_l,). 
            #   So new_t is (new_l,) and new_l is [new_t1] where new_t1 is (new_l,).
            #   So new_t[0] is new_l, and new_l[0] is new_t1, and new_t1[0] is new_l.
            #   This is a cycle: new_t -> new_l -> new_t1 -> new_l.
            #   But new_t and new_t1 are different objects. They should be the same.
            #   The problem is that we created two different tuples for the same original tuple t.
            # 
            # To fix this, we need to register the clone of t before we finish cloning its elements.
            # Since we can't create a tuple with placeholders, we can use a different approach:
            # For tuples, we can first check if it's in memo. If not, we need to create it.
            # We can create a "proxy" or use a two-pass approach.
            # 
            # Actually, a simpler approach: since tuples are immutable, we can avoid the cycle issue by 
            # noting that if a tuple is part of a cycle, the cycle must go through a mutable container.
            # So when we encounter the tuple again, we haven't registered its clone yet.
            # We can't easily fix this without a two-pass approach.
            # 
            # Let's use a two-pass approach for tuples:
            # 1. First, identify all objects that need cloning.
            # 2. Create placeholders for mutable objects.
            # 3. Fill in the placeholders.
            # 
            # But this is complex. 
            # 
            # Alternative: For tuples, we can register them in memo with a "pending" marker, and then 
            # when we encounter them again, we know we're in a cycle. But we still need to create the tuple.
            # 
            # Actually, let's look at the problem again. "Cycles must not cause infinite recursion".
            # The example given is "if obj contains itself". 
            # A tuple cannot contain itself directly. 
            # So the cycle must go through a mutable container.
            # 
            # Let's try a different approach: use a dictionary to track objects being processed.
            # If we encounter an object that is currently being processed, we have a cycle.
            # But we still need to return the correct clone.
            # 
            # I think the standard way to handle this is to use a memo that maps original to clone,
            # and for tuples, we accept that we might create multiple clones if there's a cycle,
            # but that violates the "shared references must stay shared" property.
            # 
            # Let's think about the two-pass approach more carefully.
            # Pass 1: Traverse the object graph and create empty containers for all mutable objects (lists, dicts).
            #         For tuples, we can't create empty containers, but we can note that they need to be created.
            # Pass 2: Fill in the containers.
            # 
            # For tuples, we can create them in pass 2 after all their elements are cloned.
            # But if a tuple is part of a cycle, its elements might not be fully cloned yet.
            # 
            # Actually, for the cycle t -> l -> t, where t is a tuple and l is a list:
            # Pass 1: 
            #   Start with t. t is a tuple. We can't create an empty tuple with placeholders.
            #   So we skip creating a placeholder for t.
            #   Traverse into t's elements: l.
            #   l is a list. Create empty list new_l. memo[id(l)] = new_l.
            #   Traverse into l's elements: t.
            #   t is a tuple. Skip.
            #   Traverse into t's elements: l.
            #   l is in memo. Stop.
            # Pass 2:
            #   Fill new_l: for each item in l, clone it.
            #   Clone t: t is a tuple. We need to create a new tuple with cloned elements.
            #   Clone l: l is in memo, return new_l.
            #   So new_t = (new_l,)
            #   But we need to register new_t as the clone of t.
            #   But we didn't create a placeholder for t in pass 1.
            #   So when we encounter t again, we'll create another new_t.
            # 
            # This is still problematic.
            # 
            # Let's try a different idea: for tuples, we can use a two-step process within the recursive function.
            # When we encounter a tuple, we first check if it's in memo. If not, we need to create it.
            # We can't create it immediately because we need to clone its elements first.
            # But we can register a "promise" or use a mutable container to hold the tuple.
            # 
            # Actually, I recall that for immutable containers like tuples, if they are part of a cycle,
            # the cycle must go through a mutable container. So when we encounter the tuple again via the 
            # mutable container, we haven't registered the tuple's clone yet. 
            # The key insight is that we can't have a cycle involving only immutable objects.
            # So the cycle must involve at least one mutable object.
            # 
            # Let's try this: when we encounter a tuple, we don't register it in memo until after we've 
            # cloned all its elements. But if during cloning its elements, we encounter the same tuple again,
            # we'll try to clone it again, leading to infinite recursion.
            # 
            # To break the cycle, we need to register the tuple's clone before we finish cloning its elements.
            # Since we can't create a tuple with placeholders, we can use a workaround:
            # Create a list, register it as the clone of the tuple, then convert it to a tuple later?
            # No, that doesn't work because the type would be wrong.
            # 
            # Another idea: use a custom class that acts as a placeholder for the tuple.
            # But that's overcomplicating.
            # 
            # Let's look at how the copy module handles this.
            # The copy module uses a memo dictionary. For tuples, it creates the tuple after cloning elements.
            # But it handles cycles by checking if the object is already in the memo.
            # If a tuple is part of a cycle, the copy module might fail or produce incorrect results.
            # 
            # Actually, let's test with a simple example:
            # l = []
            # t = (l,)
            # l.append(t)
            # 
            # clone(t):
            #   t not in memo.
            #   clone elements: clone(l)
            #     l not in memo.
            #     new_l = []
            #     memo[id(l)] = new_l
            #     clone(l[0]) which is t
            #       t not in memo.
            #       clone elements: clone(l)
            #         l is in memo, return new_l
            #       So we create new_t1 = (new_l,)
            #       Return new_t1.
            #     new_l.append(new_t1)
            #     Return new_l.
            #   So new_t = (new_l,)
            #   But new_l contains new_t1, which is (new_l,).
            #   So new_t is (new_l,) and new_l is [new_t1] where new_t1 is (new_l,).
            #   new_t and new_t1 are different.
            # 
            # The problem is that we created two different tuples for the same original tuple t.
            # To fix this, we need to ensure that when we encounter t the second time, we return the same clone.
            # But we haven't created the clone of t yet when we encounter it the second time.
            # 
            # The solution is to register the clone of t before we finish cloning its elements.
            # Since we can't create a tuple with placeholders, we can use a different approach:
            # For tuples, we can first create a list, register it as the clone of the tuple, 
            # then after cloning all elements, convert the list to a tuple and update the memo.
            # But the type would be wrong during the intermediate step.
            # 
            # Alternatively, we can use a two-pass approach specifically for tuples involved in cycles.
            # But detecting cycles is complex.
            # 
            # Let's try a simpler approach: use a dictionary to track objects that are currently being cloned.
            # If we encounter an object that is currently being cloned, we have a cycle.
            # In that case, we can return a placeholder or handle it specially.
            # 
            # Actually, I think the issue is that for tuples, we need to register them in memo before 
            # cloning their elements, but we can't create the tuple yet. 
            # We can create a "dummy" object and replace it later.
            # 
            # Let's use a class:
            class TuplePlaceholder:
                def __