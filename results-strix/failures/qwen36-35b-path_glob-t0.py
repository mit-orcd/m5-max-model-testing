```python
def match(pattern, path):
    """
    Returns True if a glob pattern matches a whole path.
    """
    # We will use a recursive approach with memoization or an iterative DP approach.
    # Given the constraints and potential for deep recursion with **, 
    # an iterative DP or a carefully implemented recursive function with memoization is best.
    # Let's use a recursive function with memoization on (pattern_index, path_index).
    
    # Pre-process the pattern into a list of tokens to handle ** efficiently?
    # Or just handle it in the recursion.
    # Let's stick to a direct recursive matcher with memoization.
    
    memo = {}
    
    def recurse(p_idx, h_idx):
        # p_idx: current index in pattern
        # h_idx: current index in path (host/path)
        
        state = (p_idx, h_idx)
        if state in memo:
            return memo[state]
        
        # Base cases
        if p_idx == len(pattern) and h_idx == len(path):
            memo[state] = True
            return True
        if p_idx == len(pattern):
            # Pattern exhausted, path must also be exhausted
            memo[state] = False
            return False
        if h_idx == len(path):
            # Path exhausted, pattern must be exhausted or only contain **
            # Check if remaining pattern is only **
            while p_idx < len(pattern):
                if pattern[p_idx] == '*' and p_idx + 1 < len(pattern) and pattern[p_idx+1] == '*':
                    p_idx += 2
                else:
                    memo[state] = False
                    return False
            memo[state] = True
            return True
        
        p = pattern[p_idx]
        
        # Handle **
        if p == '*' and p_idx + 1 < len(pattern) and pattern[p_idx+1] == '*':
            # It's a ** token
            # Skip the **
            next_p_idx = p_idx + 2
            
            # ** can match zero or more segments.
            # A segment is defined by not containing '/'.
            # ** matches zero or more whole segments.
            # This means ** can match:
            # 1. Empty string (zero segments) -> move to next_p_idx, keep h_idx
            # 2. One or more segments -> consume characters until next '/' or end, then move to next_p_idx
            
            # Case 1: Match zero segments
            if recurse(next_p_idx, h_idx):
                memo[state] = True
                return True
            
            # Case 2: Match one or more segments
            # We need to consume at least one character from the path, but we cannot cross a '/'
            # Actually, ** matches whole segments. So it can match "a", "a/b", "a/b/c" etc.
            # But it cannot match across a '/' unless it consumes the whole segment.
            # Wait, the definition is: "** matches zero or more whole segments".
            # So if we have "a/**/b" and path "a/x/y/b":
            # ** matches "x/y".
            # If path is "a/b", ** matches "".
            
            # So, from current h_idx, we can try to match ** with:
            # - 0 segments: recurse(next_p_idx, h_idx) [Already done above]
            # - k segments: consume k segments.
            
            # Let's iterate through possible segment endings.
            # A segment ends at '/' or end of string.
            # We can match 1 segment, then recurse.
            # We can match 2 segments, then recurse.
            # etc.
            
            # Optimization: Instead of trying every split, we can try:
            # Match 0 segments: recurse(next_p_idx, h_idx)
            # Match 1+ segments: Find the next '/' after h_idx.
            # If we match 1 segment, we consume up to the next '/' (exclusive) or end.
            # Then we recurse.
            
            # Let's find the end of the first segment starting at h_idx.
            # If path[h_idx] is '/', then the first segment is empty? 
            # Paths are separated by '/'. Usually segments are non-empty between slashes.
            # "a/b" -> segments "a", "b".
            # If h_idx is at a '/', it means we are at the start of a new segment.
            
            # Let's define: a segment is a sequence of non-'/' characters.
            # ** can match a sequence of segments.
            
            # Strategy:
            # 1. Try matching 0 segments: recurse(next_p_idx, h_idx)
            # 2. Try matching 1 or more segments:
            #    We must consume at least one segment.
            #    Find the end of the current segment.
            #    If h_idx is at '/', the current segment is empty? No, usually segments are between slashes.
            #    Let's assume standard path behavior: "a/b" has segments "a" and "b".
            #    If we are at h_idx, and path[h_idx] == '/', then we are at the boundary.
            #    Actually, if we just matched a segment, we are at a '/'.
            
            # Let's simplify:
            # ** matches zero or more segments.
            # A segment is [^/]+.
            # So ** matches (/? [^/]+)* ? No.
            # ** matches a sequence of segments.
            # If we are at h_idx, we can:
            # - Match 0 segments: stay at h_idx.
            # - Match 1 segment: consume [^/]+. If followed by '/', consume '/'. Then recurse.
            # - Match 2 segments: consume [^/]+, '/', [^/]+. If followed by '/', consume '/'. Then recurse.
            
            # This can be implemented by:
            # Try recurse(next_p_idx, h_idx) [0 segments]
            # Then, try to consume one or more segments.
            # We can do this by finding the next '/' and trying to recurse after consuming that segment.
            
            # Let's find the end of the first segment starting at h_idx.
            # If h_idx >= len(path), we can't consume any more segments.
            
            end_seg = h_idx
            while end_seg < len(path) and path[end_seg] != '/':
                end_seg += 1
            
            # Now, end_seg is either at '/' or at len(path).
            # The segment is path[h_idx:end_seg].
            
            # Option A: Match this segment and stop (if it's the last one)
            # Option B: Match this segment and the following '/' and continue matching more segments.
            
            # Actually, ** can match any number of segments.
            # So we can try:
            # 1. Match 0 segments: recurse(next_p_idx, h_idx)
            # 2. Match 1 segment: recurse(next_p_idx, end_seg) if end_seg < len(path) and path[end_seg] == '/', then recurse(next_p_idx, end_seg + 1)?
            # No, if we match 1 segment, we consume the segment. If there is a '/', it belongs to the separator.
            # If we match "a" in "a/b", we consume "a". The next char is '/'.
            # If we match "a/b", we consume "a", "/", "b".
            
            # Let's try a different approach for **:
            # ** matches zero or more segments.
            # This is equivalent to:
            # - Match 0 segments: recurse(next_p_idx, h_idx)
            # - Match 1 segment: consume [^/]+. If followed by '/', consume '/'. Then recurse(next_p_idx, new_h_idx).
            #   But wait, if we match 1 segment, we might still have more segments to match with **.
            #   So we should recurse with next_p_idx and the position after the segment (and optional '/').
            
            # Let's define a helper to consume segments.
            # But since we are in recursion, we can just try:
            # 1. recurse(next_p_idx, h_idx) [0 segments]
            # 2. If h_idx < len(path) and path[h_idx] != '/':
            #    Find end of segment.
            #    Try recurse(next_p_idx, end_seg) [