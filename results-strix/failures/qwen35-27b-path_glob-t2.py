```python
def match(pattern, path):
    def compile_pattern(p):
        tokens = []
        i = 0
        n = len(p)
        while i < n:
            c = p[i]
            if c == '\\':
                i += 1
                if i < n:
                    tokens.append(('lit', p[i]))
                else:
                    tokens.append(('lit', '\\'))
            elif c == '?':
                tokens.append(('qmark',))
            elif c == '*':
                if i + 1 < n and p[i + 1] == '*':
                    tokens.append(('doublestar',))
                    i += 1
                else:
                    tokens.append(('star',))
            elif c == '[':
                j = i + 1
                neg = False
                if j < n and p[j] == '!':
                    neg = True
                    j += 1
                if j < n and p[j] == ']':
                    j += 1
                start = j
                chars = set()
                while j < n and p[j] != ']':
                    if j + 2 < n and p[j + 1] == '-' and p[j + 2] != ']':
                        chars.update(range(ord(p[j]), ord(p[j + 2]) + 1))
                        j += 3
                    else:
                        if p[j] == '\\' and j + 1 < n:
                            chars.add(p[j + 1])
                            j += 2
                        else:
                            chars.add(p[j])
                            j += 1
                if j >= n:
                    tokens.append(('lit', '['))
                else:
                    tokens.append(('class', chars, neg))
                    i = j
            else:
                tokens.append(('lit', c))
            i += 1
        return tokens

    def match_segment(tokens, seg, t_idx, seg_idx):
        if t_idx == len(tokens):
            return seg_idx == len(seg)
        tok = tokens[t_idx]
        if tok[0] == 'lit':
            if seg_idx < len(seg) and seg[seg_idx] == tok[1]:
                return match_segment(tokens, seg, t_idx + 1, seg_idx + 1)
            return False
        elif tok[0] == 'qmark':
            if seg_idx < len(seg) and seg[seg_idx] != '/':
                return match_segment(tokens, seg, t_idx + 1, seg_idx + 1)
            return False
        elif tok[0] == 'class':
            if seg_idx < len(seg):
                c = seg[seg_idx]
                if c == '/':
                    return False
                in_class = c in tok[1]
                if tok[2]:
                    in_class = not in_class
                if in_class:
                    return match_segment(tokens, seg, t_idx + 1, seg_idx + 1)
            return False
        elif tok[0] == 'star':
            # Try matching 0 or more chars (not '/')
            for k in range(seg_idx, len(seg) + 1):
                if k > seg_idx and seg[k - 1] == '/':
                    break
                if match_segment(tokens, seg, t_idx + 1, k):
                    return True
            return False
        elif tok[0] == 'doublestar':
            # Match zero or more whole segments
            # Option 1: match nothing (0 segments)
            if match_segment(tokens, seg, t_idx + 1, seg_idx):
                return True
            # Option 2: match one or more segments
            # Find next '/' or end
            for k in range(seg_idx, len(seg) + 1):
                if k > seg_idx and seg[k - 1] == '/':
                    # Consume this segment, but must stop at segment boundary
                    if k == len(seg) or (k < len(seg) and seg[k] == '/'):
                        if match_segment(tokens, seg, t_idx, k):
                            return True
                    else:
                        # Cannot stop mid-segment for doublestar
                        continue
            return False
        return False

    # Split pattern and path into segments, but keep tokens for pattern
    path_segments = path.split('/')
    pattern_tokens = compile_pattern(pattern)

    # We need to match whole path, so process segment by segment
    # But pattern can have ** which spans segments, so we need a segment-aware matcher
    # Convert pattern into a list of segment patterns, where ** is a special token
    # Actually, let's process the entire path as a string with segment boundaries
    # But the rules say: ? and * never match '/', and ** matches whole segments
    # So we can't simply treat the whole path as one string for * and ?
    # Instead, we'll split the pattern by '/' but handle ** specially
    # However, the pattern might have ** inside a segment? No, ** is a segment itself
    # Actually, 'a/**/b' means segments: ['a', '**', 'b']
    # But what about 'a*b'? That's one segment with * inside
    # So we need to split the pattern by '/' but preserve ** as a special segment
    # And also handle escaping within segments

    # Alternative approach: process the entire path string, but enforce that
    # ? and * do not match '/', and ** matches any sequence of '/' and non-'/'
    # But ** must match whole segments, meaning it can match empty, or one segment, or multiple
    # So ** matches: empty, or (non-slash chars) + '/' + ... + (non-slash chars)
    # Actually, ** matches zero or more segments, so it can match:
    # - empty (if pattern is a/**/b and path is a/b)
    # - one segment (a/x/b)
    # - multiple segments (a/x/y/b)
    # But it cannot match a partial segment.

    # Let's re-split the pattern into segments, where ** is a special segment
    # But note: pattern might have 'a*b' which is one segment, not split by *
    # So we need to split the pattern by '/' but not by * or **
    # How to split? Split by '/' but treat ** as a segment boundary?
    # Actually, the pattern is a sequence of segments separated by '/'
    # Each segment can contain *, ?, [], etc., but ** only appears as a whole segment
    # So: split pattern by '/' -> list of segment patterns
    # But wait: what if pattern is 'a/**/b'? Splitting by '/' gives ['a', '**', 'b']
    # What if pattern is 'a*b/c'? Splitting gives ['a*b', 'c']
    # What if pattern is 'a/**b'? That's one segment: 'a/**b' -> but ** inside a segment is not special?
    # The problem says: "A path segment that is exactly '**' matches zero or more whole segments"
    # So ** only has special meaning when it is a whole segment, i.e., between slashes or at ends
    # So 'a/**b' is a segment containing '*', '*', 'b' -> which would be treated as two * and then b?
    # But the problem says: "A path segment that is exactly '**'", meaning the entire segment is '**'
    # So if a segment is '**', it's special; otherwise, * and ? are within-segment wildcards.

    # Therefore, we split the pattern by '/' to get segments, but we must be careful:
    # - If a segment is exactly '**', it's a double-star segment
    # - Otherwise, it's a normal segment with possible *, ?, etc.
    # But note: escaping: '\*' is a literal '*', so a segment could be '\*\*' which is not '**'
    # So we need to parse the pattern into segments, respecting escapes.

    # Revised plan:
    # 1. Split pattern into segments by '/', but handle escaping: a '/' is only a separator if not escaped.
    # 2. For each segment, if it is exactly '**' (after considering escapes? No, the problem says "exactly '**'", so literal)
    #    But if it's '\*\*', that's three characters: backslash, star, star -> not '**'
    #    So we need to parse