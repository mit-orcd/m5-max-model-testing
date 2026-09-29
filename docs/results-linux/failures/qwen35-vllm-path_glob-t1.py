```python
def match(pattern, path):
    def compile_pattern(p):
        # Returns a list of tokens
        # Each token is either ('LITERAL', char), ('CHAR_CLASS', list), ('MATCH_SINGLE',), ('MATCH_ZERO',), ('MATCH_MULTI',)
        tokens = []
        i = 0
        n = len(p)
        while i < n:
            c = p[i]
            if c == '?':
                tokens.append(('MATCH_SINGLE',))
                i += 1
            elif c == '*':
                if i + 1 < n and p[i+1] == '*':
                    # Check if it's a whole segment match (surrounded by / or start/end)
                    # We need to look ahead to see if it's followed by /
                    # If we have ** followed by /, it's a multi-segment matcher
                    # If we have ** at end, it's also multi-segment
                    # If we have * followed by non-star, it's single star
                    # If we have ** at start, it's multi-segment
                    # We need to determine if this ** represents a "whole segment" match
                    # The spec says "A path segment that is exactly '**' matches zero or more whole segments"
                    # This implies the pattern segment is literally "**"
                    
                    # Check boundaries
                    is_start = (i == 0) or (p[i-1] == '/')
                    is_end = (i + 2 >= n) or (p[i+2] == '/')
                    
                    if is_start and is_end:
                        tokens.append(('MATCH_MULTI',))
                        i += 2
                        # Skip the trailing slash if it was part of the segment boundary check?
                        # No, the slash is a separator, not part of the token.
                        # If p is "a/**/b", i is at first *. p[i+1] is *.
                        # is_start: p[i-1] is 'a' -> not start of segment? Wait.
                        # We need to parse segments.
                        # Let's re-evaluate: The pattern is a string. We need to split by '/' but handle ** specially.
                        # Actually, let's just handle the logic here.
                        # If we have ** and it's surrounded by / or boundaries, it's a multi-segment wildcard.
                        # But what if it's "a/**"? i=1, i-1='a' (not /), so not start of segment?
                        # The rule is "A path segment that is exactly '**'".
                        # So we must parse the pattern into segments first.
                        pass
                    else:
                        # It's not a standalone ** segment, treat as normal * (but this case shouldn't happen if we parse segments)
                        # Actually, simpler approach: parse segments, then process each segment.
                        pass
                else:
                    tokens.append(('MATCH_ZERO',))
                    i += 1
            elif c == '[':
                # Parse character class
                j = i + 1
                neg = False
                if j < n and p[j] == '!':
                    neg = True
                    j += 1
                chars = set()
                while j < n and p[j] != ']':
                    if j + 2 < n and p[j+1] == '-' and p[j+2] != ']':
                        # Range
                        start = p[j]
                        end = p[j+2]
                        for k in range(ord(start), ord(end) + 1):
                            chars.add(chr(k))
                        j += 3
                    else:
                        chars.add(p[j])
                        j += 1
                if j < n:
                    j += 1 # Skip ]
                tokens.append(('CHAR_CLASS', chars, neg))
                i = j
            elif c == '\\':
                if i + 1 < n:
                    tokens.append(('LITERAL', p[i+1]))
                    i += 2
                else:
                    tokens.append(('LITERAL', '\\'))
                    i += 1
            else:
                tokens.append(('LITERAL', c))
                i += 1
        return tokens

    # Better approach: Split pattern by '/' but handle ** as a special segment
    # Segments can be: normal string, '*', '**', or character classes
    
    def parse_segments(p):
        segments = []
        current = []
        i = 0
        n = len(p)
        while i < n:
            c = p[i]
            if c == '/':
                if not current:
                    segments.append('') # Empty segment
                else:
                    segments.append(''.join(current))
                current = []
                i += 1
            elif c == '*':
                if i + 1 < n and p[i+1] == '*':
                    # Check if it's exactly **
                    # If next char is / or end, it's **
                    # If prev char is / or start, it's **
                    # We are inside a segment build.
                    # If we see **, we need to check if it's the whole segment.
                    # Since we are building a segment, if we encounter **, we should check boundaries.
                    # But we are building 'current'.
                    # Let's handle ** detection during the loop.
                    # If we see **, and it's surrounded by / (or start/end of string), it's a multi-segment match.
                    # But we are splitting by /.
                    # So if the segment we are building is exactly "**", then it's a multi-segment.
                    # How do we know if it's exactly "**"?
                    # If the next chars are "**" and then "/" or end, and previous was "/" or start.
                    # Let's just accumulate chars. If the accumulated string is "**", we treat it specially.
                    # But wait, what if it's "a**b"? That's not a segment of exactly "**".
                    # So we need to detect if the segment is *exactly* "**".
                    # We can do this after splitting? No, we need to split carefully.
                    pass
                else:
                    current.append(c)
                    i += 1
            elif c == '\\':
                if i + 1 < n:
                    current.append(p[i+1])
                    i += 2
                else:
                    current.append('\\')
                    i += 1
            else:
                current.append(c)
                i += 1
        if current:
            segments.append(''.join(current))
        else:
            # If pattern ends with /, we have an empty segment at the end?
            # e.g. "a/" -> segments ["a", ""]
            pass
        return segments

    # Revised parsing to handle ** correctly
    def get_tokens(p):
        tokens = []
        i = 0
        n = len(p)
        while i < n:
            c = p[i]
            if c == '/':
                tokens.append(('SEGMENT',))
                i += 1
            elif c == '*':
                if i + 1 < n and p[i+1] == '*':
                    # Check if this is exactly **
                    # Look back
                    is_start = (i == 0) or (p[i-1] == '/')
                    # Look forward
                    is_end = (i + 2 >= n) or (p[i+2] == '/')
                    if is_start and is_end:
                        tokens.append(('MATCH_MULTI',))
                        i += 2
                        continue
                    else:
                        # Not a standalone **, treat as literal * or part of segment?
                        # The problem says "A path segment that is exactly '**'".
                        # So if it's not a segment of exactly **, it's just two * characters?
                        # No, in glob, ** inside a segment is not standard unless it's a whole segment.
                        # But the spec says "A path segment that is exactly '**' matches zero or more whole segments".
                        # This implies if the segment is not exactly "**", it doesn't get this special behavior.
                        # So "a**b" is a literal segment with two stars.
                        # So we should treat * as MATCH_ZERO unless it's ** as a segment.
                        # But our loop is processing character by character.
                        # If we find ** not as a segment, we should treat them as two MATCH_ZERO tokens?
                        # Or just one? The spec says "**" is a segment.
                        # So if we are not in a segment context (i.e. between /), we check for **.
                        # If we are inside a segment (e.g. "a*"), we treat * as MATCH_ZERO.
                        # How to know if we are between /?
                        # We can just check the context.
                        # If the previous char was / (or start) and next is / (or end), it's a segment.
                        # So if we see ** and context is segment, it's MATCH_MULTI.
                        # Otherwise, it's two * characters? Or one *?
                        # In standard glob, ** is a special token.
                        # Let's assume if it's not a whole segment, we treat it as normal * (MATCH_ZERO) or literal?
                        # The spec says "A path segment that is exactly '**' matches...".
                        # It doesn't say what happens if it's not.
                        # But logically, if it's not a whole segment, it's just characters.
                        # So "a**b" -> 'a', MATCH_ZERO, MATCH_ZERO, 'b'? Or 'a', '*', '*', 'b'?
                        # Let's assume if it's not a whole segment, we treat * as MATCH_ZERO.
                        # But wait, if we have "a**", and it's at the end, it's a segment of "**".
                        # So "a**" -> segment "a**" -> not exactly "**", so it's literal *?
                        # No, the segment is "a**". It's not exactly "**".
                        # So the stars are just stars.
                        # But stars have special meaning.
                        # In standard glob, ** is only special if it's a whole segment.
                        # If it's "a**", it's not special. So it's "a" then two * (MATCH_ZERO).
                        # But "a**" as a segment?
                        # Let's simplify:
                        # If we encounter ** and it is surrounded by / or boundaries, it's MATCH_MULTI.
                        # Otherwise, treat * as MATCH_ZERO.
                        # So "a**" -> 'a', MATCH_ZERO, MATCH_ZERO?
                        # Or is "a**" a segment?
                        # The parser splits by /.
                        # So "a**" is one segment.
                        # We need to check if the segment is exactly "**".
                        # So we should parse segments first, then check if segment == "**".
                        pass
                else:
                    tokens.append(('MATCH_ZERO',))
                    i += 1
                    continue
            elif c == '[':
                j = i + 1
                neg = False
                if j < n and p[j] == '!':
                    neg = True
                    j += 1
                chars = set()
                while j < n and p[j] != ']':
                    if j + 2 < n and p[j+1] == '-' and p[j+2] != ']':
                        start = p[j]
                        end = p[j+2]
                        for k in range(ord(start), ord(end) + 1):
                            chars.add(chr(k))
                        j += 3
                    else:
                        chars.add(p[j])
                        j += 1
                if j < n:
                    j += 1
                tokens.append(('CHAR_CLASS', chars, neg))
                i = j
            elif c == '\\':
                if i + 1 < n:
                    tokens.append(('LITERAL', p[i+1]))
                    i += 2
                else:
                    tokens.append(('LITERAL', '\\'))
                    i += 1
            else:
                tokens.append(('LITERAL', c))
                i += 1
        
        # Now we need to handle the ** case properly.
        # The above logic missed the ** check.
        # Let's rewrite the loop to handle ** segments.
        pass

    # Let's try a different approach: Split pattern into segments by '/', then process each segment.
    # But we must preserve the ** logic.
    
    # Split pattern by '/', but keep track of ** segments.
    # We can split the pattern string by '/' and check each segment.
    raw_segments = pattern.split('/')
    segments = []
    for seg in raw_segments:
        if seg == '**':
            segments.append(('MATCH_MULTI',))
        else:
            # Parse the segment into tokens
            seg_tokens = []
            i = 0
            n = len(seg)
            while i < n:
                c = seg[i]
                if c == '?':
                    seg_tokens.append(('MATCH_SINGLE',))
                    i += 1
                elif c == '*':
                    seg_tokens.append(('MATCH_ZERO',))
                    i += 1
                elif c == '[':
                    j = i + 1
                    neg = False
                    if j < n and seg[j] == '!':
                        neg = True
                        j += 1
                    chars = set()
                    while j < n and seg[j] != ']':
                        if j + 2 < n and seg[j+1] == '-' and seg[j+2] != ']':
                            start = seg[j]
                            end = seg[j+2]
                            for k in range(ord(start), ord(end) + 1):
                                chars.add(chr(k))
                            j += 3
                        else:
                            chars.add(seg[j])
                            j += 1
                    if j < n:
                        j += 1
                    seg_tokens.append(('CHAR_CLASS', chars, neg))
                    i = j
                elif c == '\\':
                    if i + 1 < n:
                        seg_tokens.append(('LITERAL', seg[i+1]))
                        i += 2
                    else:
                        seg_tokens.append(('LITERAL', '\\'))
                        i += 1
                else:
                    seg_tokens.append(('LITERAL', c))
                    i += 1
            segments.append(('SEGMENT', seg_tokens))
    
    # Now split path by '/'
    path_segments = path.split('/') if path else []
    # Handle empty path
    if path == "":
        if pattern == "":
            return True
        # If pattern is not empty, can it match empty path?
        # Only if pattern is "**" or similar?
        # But path_segments is empty.
        # We need to check if pattern can match empty.
        # If pattern is "**", it matches empty.
        # But our segments list for "**" is [('MATCH_MULTI',)].
        # So we need to handle empty path.
        pass

    # Recursive match function
    def match_segments(p_idx, p_seg_idx, path_idx, path_seg_idx):
        # p_idx: index in raw_segments (to handle ** logic? No, we already processed)
        # Actually, we have a list of segments (mixed MATCH_MULTI and SEGMENT tokens)
        # But we need to handle the case where ** matches zero or more segments.
        # Our segments list is a list of: ('MATCH_MULTI',) or ('SEGMENT', tokens)
        
        # We need to match p_segments against path_segments.
        # path_segments is a list of strings.
        # p_segments is a list of segment matchers.
        
        # Let's flatten the p_segments into a list of matchers.
        # But ** is a matcher that can span multiple path segments.
        
        # Let's use a recursive function with index into p_segments and path_segments.
        
        # p_list: list of matchers (either ('MATCH_MULTI',) or ('SEGMENT', tokens))
        # path_list: list of path segments (strings)
        
        # We need to match p_list[0:] against path_list[0:]
        
        if not p_segments:
            return path_idx == len(path_segments)
        
        if not path_segments and path_idx == len(path_segments):
            # Path is exhausted
            # Check if remaining pattern can match empty
            # If remaining is all ** or empty segments?
            # If p_segments[0] is MATCH_MULTI, it can match empty.
            # Then we can skip it and continue.
            # If p_segments[0] is SEGMENT, it cannot match empty (unless empty segment? No, segments have tokens)
            # Wait, if path is empty, and pattern is "**", it matches.
            # If pattern is "a", it doesn't match.
            # So we need to check if the rest of pattern can match empty path.
            # Only ** can match empty path (by matching zero segments).
            # So if all remaining p_segments are MATCH_MULTI, return True.
            # Otherwise False.
            return all(p == ('MATCH_MULTI',) for p in p_segments)
        
        # If path is exhausted but pattern is not, and we have a SEGMENT, fail.
        if path_idx == len(path_segments):
            return False
        
        current_p = p_segments[p_seg_idx]
        current_path = path_segments[path_idx]
        
        if current_p == ('MATCH_MULTI',):
            # ** matches zero or more segments.
            # Option 1: Match zero segments.
            # Then we must match the rest of pattern against current path.
            # But wait, if we match zero segments, we stay at current path.
            # So we call match_segments(p_idx+1, path_idx)
            if match_segments(p_seg_idx + 1, path_idx):
                return True
            # Option 2: Match one or more segments.
            # We consume one or more path segments and try to match the rest.
            # We can iterate over how many segments to consume.
            # But we must ensure that the rest of the pattern can match the remaining path.
            # We can try to match the current path segment against the next pattern segment?
            # No, ** can match multiple segments.
            # So we can try to match the current path segment against the next pattern segment (if it exists).
            #