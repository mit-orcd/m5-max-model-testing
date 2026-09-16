def match(pattern, path):
    def escape_char(s):
        result = []
        i = 0
        while i < len(s):
            if s[i] == '\\' and i + 1 < len(s):
                result.append(s[i+1])
                i += 2
            else:
                result.append(s[i])
                i += 1
        return ''.join(result)

    def char_in_class(ch, class_str):
        if not class_str:
            return False
        negated = False
        idx = 0
        if class_str[0] in ('!', '^'):
            negated = True
            idx = 1
        
        if idx >= len(class_str) - 1:
            return False
            
        match_found = False
        while idx < len(class_str):
            start = class_str[idx]
            if idx + 2 < len(class_str) and class_str[idx+1] == '-' and class_str[idx+2] != ']':
                end = class_str[idx+2]
                if start <= ch <= end:
                    match_found = True
                idx += 3
            else:
                if start == ch:
                    match_found = True
                idx += 1
        
        return not match_found if negated else match_found

    def match_segment(seg, pat_seg):
        if pat_seg == '**':
            return True
        if pat_seg == '*':
            return len(seg) == 0 or (len(seg) > 0 and all(c != '/' for c in seg))
        
        i = 0
        j = 0
        while i < len(seg) and j < len(pat_seg):
            p = pat_seg[j]
            if p == '?':
                if seg[i] == '/':
                    return False
                i += 1
                j += 1
            elif p == '*':
                # * matches any sequence except /
                k = i
                while k <= len(seg) and (k == len(seg) or seg[k] != '/'):
                    if match_segment(seg[k:], pat_seg[j+1:]):
                        return True
                    k += 1
                return False
            elif p == '[':
                bracket_end = pat_seg.find(']', j+1)
                if bracket_end == -1:
                    return False
                class_str = pat_seg[j+1:bracket_end]
                if not char_in_class(seg[i], class_str):
                    return False
                i += 1
                j = bracket_end + 1
            elif p == '\\':
                if j + 1 >= len(pat_seg):
                    return False
                if seg[i] != pat_seg[j+1]:
                    return False
                i += 1
                j += 2
            else:
                if seg[i] != p:
                    return False
                i += 1
                j += 1
        
        while j < len(pat_seg):
            p = pat_seg[j]
            if p == '*':
                j += 1
            else:
                return False
        
        return i == len(seg)

    # Split pattern and path into segments
    path_segs = path.split('/')
    
    # Handle pattern with **
    pat_parts = []
    i = 0
    while i < len(pattern):
        if pattern[i:i+2] == '**':
            pat_parts.append('**')
            i += 2
        elif pattern[i] == '/':
            i += 1
        else:
            seg_start = i
            while i < len(pattern) and pattern[i] != '/':
                if pattern[i:i+2] == '**':
                    break
                i += 1
            if seg_start < i:
                pat_parts.append(pattern[seg_start:i])
    
    # Remove empty segments from path if path starts or ends with /
    # But the problem says paths are separated by '/', so we assume normalized paths
    # Actually, let's handle the splitting more carefully
    path_segs = [s for s in path.split('/') if s != '']
    if path == '':
        path_segs = []
    elif path.startswith('/') and not path.endswith('/') and len(path) > 1:
        # Leading slash means first segment is empty, but we filtered empty
        pass
    elif path.endswith('/') and not path.startswith('/') and len(path) > 1:
        # Trailing slash means last segment is empty
        pass
    # Actually, for simplicity, let's re-split considering the structure
    # If path is "a/b", split gives ['a', 'b']
    # If path is "", split gives ['']
    # If path is "/", split gives ['', '']
    
    # Let's normalize path segments
    if path == '':
        path_segs = []
    else:
        path_segs = path.split('/')
        # Filter out empty strings except if the path is just "/" or similar
        # But the problem says "whole path", so we need to be careful
        # Let's keep all segments including empty ones for now, but handle them
        # Actually, standard behavior: "a/b" -> ['a','b'], "" -> [], "/" -> ['','']
        # But our matching logic expects non-empty segments for glob patterns
        # Let's assume paths don't have leading/trailing slashes except for root
        # And pattern doesn't have them either unless specified
        # For simplicity, let's split and filter empty segments
        path_segs = [s for s in path_segs if s != '']
        if path == '/':
            path_segs = [] # Root path is empty segments? Or one empty segment?
            # Actually, "/" should match pattern "/" or ""?
            # Let's assume "/" is represented as empty list of segments after filtering
            # But that might not be correct. Let's reconsider.
            # If path is "/", split gives ['', ''], filtering gives []
            # If pattern is "*", it should not match "/" because * doesn't match /
            # So we need to handle the root case specially or keep the segments
            # Let's not filter empty segments and handle them in matching
            pass
    
    # Better approach: don't filter, but handle empty segments
    # Actually, the problem states: "Paths are separated by '/'"
    # So "a/b" has two segments: "a" and "b"
    # "" has zero segments
    # "/" has two empty segments? Or one?
    # Let's assume standard split: "a/b".split('/') -> ['a','b']
    # "/a/b".split('/') -> ['', 'a', 'b']
    # "a/b/".split('/') -> ['a', 'b', '']
    # We'll keep all segments and handle empty ones
    
    path_segs = path.split('/')
    if path == '':
        path_segs = []
    else:
        path_segs = path.split('/')
        # If path ends with '/', the last element is empty string
        # We'll keep it
    
    # Now, we need to match the pattern against the path segments
    # But pattern may have ** which can match multiple segments
    # So we can't just split pattern by '/' and match segment by segment
    
    # Alternative: use dynamic programming or recursion on segments
    # Let's use recursion with memoization
    
    def match_segments(pat_segs, path_segs, p_idx, path_idx):
        if p_idx == len(pat_segs) and path_idx == len(path_segs):
            return True
        if p_idx == len(pat_segs):
            return False
        if path_idx == len(path_segs):
            # If remaining pattern is all '**', then it's ok
            for k in range(p_idx, len(pat_segs)):
                if pat_segs[k] != '**':
                    return False
            return True
        
        p_seg = pat_segs[p_idx]
        
        if p_seg == '**':
            # Skip this '**' and try matching remaining pattern with current or future path segments
            # Option 1: skip this '**' (match zero segments)
            if match_segments(pat_segs, path_segs, p_idx+1, path_idx):
                return True
            # Option 2: match one or more segments
            if path_idx < len(path_segs):
                if match_segments(pat_segs, path_segs, p_idx, path_idx+1):
                    return True
            return False
        
        # Current pattern segment must match current path segment
        if p_seg == '*':
            # * matches entire segment but not /, so it matches exactly one path segment
            # But wait, * in pattern segment means it matches the whole segment
            # Actually, in our segmentation, * is a segment that matches any single segment
            # But the problem says: '*' matches zero or more characters but never '/'
            # So in a segment context, * matches the entire segment (which has no '/')
            # But what if pattern is "a/*/b"? Then the middle * is a segment that matches any segment
            # However, if pattern is "a* b", then * is part of a segment
            # Our segmentation splits by '/', so * as a segment means it matches any single segment
            # But the problem also allows * within a segment, like "a*b"
            # So we need to handle * within segments too
            
            # Actually, our initial segmentation is flawed because * and ? and [] are within segments
            # We should not split pattern by '/' if it contains glob characters
            # Instead, we should split both pattern and path by '/' and then match segment by segment,
            # but handle the glob characters within each segment
            
            # Let's redefine: split pattern and path by '/', then for each segment, 
            # use the match_segment function defined earlier, which handles *, ?, [], etc.
            # But ** is special: it matches zero or more whole segments
            
            # So we need a different approach:
            # - Split pattern by '/' but keep ** as a separate segment
            # - Split path by '/'
            # - Then match segments, where ** can match multiple path segments
            
            # How to split pattern: 
            # "a/**/b" -> ['a', '**', 'b']
            # "a*b" -> ['a*b'] (one segment)
            # "a?b" -> ['a?b']
            # "[abc]" -> ['[abc]']
            
            # So we need a function to split pattern into segments, treating ** as a special segment
            
            pass # We'll handle this below
        
        # For now, assume p_seg is a single segment pattern (without **)
        if p_seg == '*':
            # * as a segment matches any single path segment (since * doesn't match /)
            # But wait, the problem says: '*' matches zero or more characters but never '/'
            # So in a segment context, * matches the entire segment (which is non-empty? or empty?)
            # Actually, * can match empty string too? "a*b" matches "ab" (b is empty)
            # But in segment matching, if pattern segment is "*", it should match any segment (including empty?)
            # However, path segments from split might be empty if there are consecutive slashes or leading/trailing
            # Let's assume path segments can be empty
            
            # But our match_segment function handles * within a segment, not as a whole segment
            # So we need to adjust: if p_seg is "*", it matches any path segment (including empty)
            # But the problem says * matches zero or more characters, so it can match empty segment
            return True # because any segment (even empty) can be matched by *
        
        # If p_seg contains glob characters, we need to match it against path_segs[path_idx]
        # But p_seg might be something like "a*b", which should match a path segment that fits
        # So we use match_segment for this
        
        if p_seg == '**':
            # Already handled above
            pass
        
        # For non-** segments, they must match exactly one path segment
        if path_idx >= len(path_segs):
            return False
        
        current_path_seg = path_segs[path_idx]
        
        # If p_seg is a glob pattern (contains *, ?, [], etc.), use match_segment
        # But note: p_seg might be exactly "*" or "**", but we handled "*" above and "**" is separate
        # Actually, we need to distinguish:
        # - If p_seg is "**", it's a special segment that matches multiple path segments
        # - If p_seg is "*", it's a segment that matches any single path segment (including empty)
        # - If p_seg contains other glob chars, use match_segment
        
        # But wait, our segmentation might have split "a/**/b" into ['a','**','b'], so p_seg='**' is handled
        # And "a*b" is one segment, so p_seg='a*b', which we match against current_path_seg using match_segment
        
        # However, what if p_seg is exactly '*'? Then it should match any single path segment
        # But match_segment('*') would also work if we design it to handle '*' as a whole segment
        
        # Let's adjust: if p_seg is '*', then it matches any path segment (including empty)
        if p_seg == '*':
            # Match any single path segment
            return match_segments(pat_segs, path_segs, p_idx+1, path_idx+1)
        
        # If p_seg contains glob characters, use match_segment
        if '*' in p_seg or '?' in p_seg or '[' in p_seg or '\\' in p_seg:
            if not match_segment(current_path_seg, p_seg):
                return False
            return match_segments(pat_segs, path_segs, p_idx+1, path_idx+1)
        else:
            # Literal segment
            if p_seg != current_path_seg:
                return False
            return match_segments(pat_segs, path_segs, p_idx+1, path_idx+1)

    # Split pattern into segments, treating ** as a separate segment
    def split_pattern(pattern):
        segments = []
        i = 0
        n = len(pattern)
        while i < n:
            if pattern[i:i+2] == '**':
                segments.append('**')
                i += 2
            elif pattern[i] == '/':
                i += 1
            else:
                start = i
                while i < n and pattern[i] != '/':
                    if pattern[i:i+2] == '**':
                        break
                    i += 1
                if start < i:
                    segments.append(pattern[start:i])
        return segments

    pat_segs = split_pattern(pattern)
    path_segs = path.split('/') if path != '' else []
    
    # Special case: if path is empty string, path_segs is []
    # If pattern is empty, pat_segs is []
    
    return match_segments(pat_segs, path_segs, 0, 0)