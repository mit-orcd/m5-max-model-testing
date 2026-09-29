```python
def match(pattern, path):
    def compile_pattern(p):
        tokens = []
        i = 0
        n = len(p)
        while i < n:
            c = p[i]
            if c == '\\':
                if i + 1 < n:
                    tokens.append(('lit', p[i + 1]))
                    i += 2
                else:
                    tokens.append(('lit', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('qmark',))
                i += 1
            elif c == '*':
                if i + 1 < n and p[i + 1] == '*':
                    tokens.append(('starstar',))
                    i += 2
                else:
                    tokens.append(('star',))
                    i += 1
            elif c == '[':
                j = i + 1
                neg = False
                if j < n and p[j] == '!':
                    neg = True
                    j += 1
                if j < n and p[j] == ']':
                    j += 1
                chars = set()
                while j < n and p[j] != ']':
                    if p[j] == '\\' and j + 1 < n:
                        chars.add(p[j + 1])
                        j += 2
                    elif j + 2 < n and p[j + 1] == '-' and p[j + 2] != ']':
                        chars.update(range(ord(p[j]), ord(p[j + 2]) + 1))
                        j += 3
                    else:
                        chars.add(p[j])
                        j += 1
                if j >= n:
                    tokens.append(('lit', '['))
                    i += 1
                else:
                    tokens.append(('class', chars, neg))
                    i = j + 1
            else:
                tokens.append(('lit', c))
                i += 1
        return tokens

    def match_segment(seg_tokens, seg):
        i = 0
        j = 0
        n = len(seg_tokens)
        m = len(seg)
        while i < n and j < m:
            t = seg_tokens[i]
            if t[0] == 'lit':
                if seg[j] != t[1]:
                    return False
                i += 1
                j += 1
            elif t[0] == 'qmark':
                if seg[j] == '/':
                    return False
                i += 1
                j += 1
            elif t[0] == 'star':
                # * matches zero or more non-/ chars
                # Try matching zero first, then one, etc.
                # But since we are in a segment, we can just skip ahead
                # Actually, we need to try all possibilities? No, greedy is fine for * in segment
                # But wait, * is non-greedy in regex sense? No, we need to try all splits.
                # However, since * cannot match /, and we are inside a segment, we can just match as much as possible?
                # Actually, no: consider pattern "a*b" and segment "acb". * matches "c".
                # But if pattern is "a*b" and segment "ab", * matches empty.
                # We need to try all possible lengths for *.
                # But since we are in a segment, and * cannot match /, we can just try all splits.
                # However, to avoid exponential time, we can use DP or memoization.
                # But the problem says "whole path", and segments are separated by /.
                # Since * cannot match /, it is confined to the current segment.
                # So we can just try all possible splits for * in the segment.
                # But that could be O(n^2) per segment. Given constraints, it's acceptable.
                # Actually, we can do it greedily? No, because later tokens might require specific chars.
                # Example: pattern "a*b", segment "acb". If * matches "ac", then 'b' matches 'b'. Good.
                # But if pattern "a*b", segment "ab", * matches empty.
                # We need to try all possibilities.
                # However, we can use recursion with memoization for the segment matching.
                # But let's do iterative with backtracking? Or use DP.
                # Since the segment is small, we can do recursion.
                # But to keep it simple, we'll use a helper function for segment matching with memoization.
                # Actually, we'll do it in the main function with memoization for the whole path.
                # But for now, let's assume we can do it without memoization for segments? 
                # No, worst case could be bad. But the problem doesn't specify constraints.
                # We'll implement a simple recursive matcher for segments with memoization.
                pass
            elif t[0] == 'class':
                chars, neg = t[1], t[2]
                if seg[j] == '/':
                    return False
                in_class = seg[j] in chars
                if neg:
                    in_class = not in_class
                if not in_class:
                    return False
                i += 1
                j += 1
            else:
                return False
        # After loop, if we have consumed all tokens and all segment chars, success.
        # But we might have leftover tokens that are * or qmark? No, because we are in segment matching.
        # Actually, the segment tokens should be fully consumed and segment fully consumed.
        # But wait, we might have * that matches zero or more, so we need to handle that.
        # The above loop doesn't handle * properly. We need to handle * by trying all splits.
        # So we'll change the approach: for segment matching, we'll use a recursive function with memoization.
        return False

    # Instead, let's do the whole matching with a recursive function that handles segments and **
    # We'll split the pattern and path by '/', but ** is special.
    # Actually, we can process the pattern tokens and path segments together.

    # Let's restructure: 
    # We'll have a function that matches pattern tokens against path segments.
    # But ** can match multiple segments, so we need to handle that.

    # Alternative approach: 
    # Convert pattern to a list of tokens, and path to a list of segments.
    # Then use a recursive matcher that handles:
    # - lit, qmark, class: match one segment char by char? No, because * and ** are at segment level.
    # Actually, the pattern can have * and ** that span segments? No, * cannot match /, so * is within a segment.
    # ** matches zero or more whole segments.

    # So we can split the pattern by '/' but be careful with ** and *.
    # Actually, the pattern tokens we compiled are at the character level, but we need to handle segments.

    # Let's do: 
    # Split the path into segments: path_segments = path.split('/')
    # But the pattern might have ** which matches multiple segments.

    # We'll use a recursive function that takes:
    # - pattern_tokens: list of tokens (from compile_pattern)
    # - path_segments: list of segments (from path.split('/'))
    # - seg_idx: current segment index in path_segments
    # - char_idx: current character index in the current segment (if we are in the middle of a segment)

    # But this is complicated because * and ** are at different levels.

    # Better approach: 
    # Since * cannot match /, it is confined to a single segment.
    # ** matches zero or more whole segments.
    # So we can split the pattern into "segment patterns" separated by '/' or '**'.
    # But ** is a special token.

    # Let's recompile the pattern into a list of "segment patterns", where each segment pattern is a list of tokens that must match a single segment.
    # And ** is a special marker that matches zero or more segments.

    # Steps:
    # 1. Split the pattern by '/' but keep the separators? No, because ** is two stars.
    # Actually, we can parse the pattern into a list of items, where each item is either:
    #   - a segment pattern (list of tokens for one