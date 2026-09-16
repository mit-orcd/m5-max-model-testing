```python
def match(pattern, path):
    def compile_pattern(p):
        # Returns a list of tokens: ('LITERAL', char), ('QMARK'), ('STAR'), ('STARSTAR'), ('CLASS', chars), ('ESCAPED', char)
        tokens = []
        i = 0
        n = len(p)
        while i < n:
            c = p[i]
            if c == '\\':
                if i + 1 < n:
                    tokens.append(('ESCAPED', p[i+1]))
                    i += 2
                else:
                    tokens.append(('ESCAPED', '\\'))
                    i += 1
            elif c == '?':
                tokens.append(('QMARK',))
                i += 1
            elif c == '*':
                if i + 1 < n and p[i+1] == '*':
                    # Check if it's a standalone ** (surrounded by / or start/end)
                    # We need to handle ** carefully.
                    # If we see **, we consume both.
                    # But we must ensure it's not part of a longer star sequence like *** (though spec says ** matches segments)
                    # The spec says "exactly '**' matches zero or more whole segments".
                    # So '***' is likely not a valid pattern for segment matching, but let's assume standard behavior:
                    # If we have **, we treat it as STARSTAR.
                    # If we have a single *, it's STAR.
                    # If we have ***, the first two are STARSTAR, the third is STAR? Or is it invalid?
                    # Usually in glob, *** is treated as ** followed by *.
                    # Let's implement strictly: if we see **, consume both.
                    tokens.append(('STARSTAR',))
                    i += 2
                    # Check if there is another * immediately after
                    if i < n and p[i] == '*':
                        tokens.append(('STAR',))
                        i += 1
                else:
                    tokens.append(('STAR',))
                    i += 1
            elif c == '[':
                # Parse character class
                j = i + 1
                negated = False
                if j < n and p[j] == '!':
                    negated = True
                    j += 1
                if j < n and p[j] == ']':
                    j += 1
                # Collect characters
                chars = set()
                while j < n and p[j] != ']':
                    if p[j] == '\\' and j + 1 < n:
                        chars.add(p[j+1])
                        j += 2
                    elif j + 2 < n and p[j+1] == '-' and p[j+2] != ']':
                        # Range
                        start = p[j]
                        end = p[j+2]
                        # Handle escaped start/end? Usually ranges don't escape inside, but let's be safe.
                        # Spec says backslash escapes next char.
                        # If we are inside a class, does backslash escape? Yes.
                        # But ranges are tricky. Let's assume standard behavior: [a-z]
                        # If we see \- it's a literal hyphen.
                        # If we see a-z, it's a range.
                        # We need to handle the case where the range is invalid or end is escaped.
                        # Simple approach: if p[j+1] is '-', check if p[j+2] is not ']'
                        # If p[j+2] is escaped, it's a literal.
                        # Let's just collect the range characters.
                        # We need to handle the case where the range is like [a-] or [-z]
                        # For simplicity, we'll assume valid ranges as per spec.
                        # If the character after '-' is ']', it's a literal hyphen.
                        # If it's escaped, it's a literal.
                        # Let's re-parse carefully.
                        # Actually, let's just iterate and build the set.
                        # If we see a range, expand it.
                        # We need to know if the '-' is a range operator or literal.
                        # It is a range operator if it is between two characters and not at start/end.
                        # But we are inside the loop.
                        # Let's switch strategy: parse the class into a list of (char, is_range_end) or just expand.
                        # Better: parse the class content into a list of items (char or range).
                        pass
                    j += 1
                # Re-implement class parsing properly
                class_start = i
                i = j + 1 # Move past ']'
                # Re-scan from class_start to find the class content
                # Reset i to class_start + 1
                i = class_start + 1
                content_start = i
                if i < n and p[i] == '!':
                    i += 1
                if i < n and p[i] == ']':
                    i += 1
                # Find the closing bracket
                while i < n and p[i] != ']':
                    i += 1
                if i >= n:
                    # No closing bracket, treat '[' as literal
                    tokens.append(('LITERAL', '['))
                    i = class_start + 1
                    continue
                
                class_content = p[class_start+1:i]
                # Parse class_content
                class_chars = set()
                k = 0
                neg = False
                if k < len(class_content) and class_content[k] == '!':
                    neg = True
                    k += 1
                while k < len(class_content):
                    if k + 2 < len(class_content) and class_content[k+1] == '-' and class_content[k+2] != ']':
                        # Range
                        start_char = class_content[k]
                        end_char = class_content[k+2]
                        # Handle escapes? The spec says backslash escapes next char.
                        # If start_char is escaped, it's a literal.
                        # But we are parsing the content string.
                        # Let's assume the input string has already handled escapes?
                        # No, we are parsing the raw string.
                        # If we see \-, it's a literal hyphen.
                        # If we see a-z, it's a range.
                        # We need to check if the characters are escaped.
                        # Actually, the backslash handling is done at the top level.
                        # But inside [], backslash might be literal or escape.
                        # Spec: "A backslash escapes the next character so it is treated literally."
                        # This applies everywhere.
                        # So if we see \-, it's a literal hyphen.
                        # If we see a-z, it's a range.
                        # We need to handle this in the class parsing.
                        # Let's restart the class parsing logic inside the loop.
                        pass
                    k += 1
                # Let's rewrite the class parsing cleanly
                # Reset i to class_start + 1
                i = class_start + 1
                if i < n and p[i] == '!':
                    i += 1
                if i < n and p[i] == ']':
                    i += 1
                # Find end
                end_idx = i
                while end_idx < n and p[end_idx] != ']':
                    end_idx += 1
                if end_idx >= n:
                    tokens.append(('LITERAL', '['))
                    i = class_start + 1
                    continue
                
                content = p[i:end_idx]
                # Parse content
                chars = set()
                k = 0
                negated = False
                if k < len(content) and content[k] == '!':
                    negated = True
                    k += 1
                while k < len(content):
                    if k + 2 < len(content) and content[k+1] == '-' and content[k+2] != ']':
                        # Range
                        # Check if start or end is escaped?
                        # The content string is raw.
                        # If content[k] is '\', then it's an escaped char.
                        # But we are looking for a range.
                        # If content[k] is '\', then content[k+1] is the char.
                        # Then content[k+2] must be '-'?
                        # This is getting complex.
                        # Let's assume standard glob behavior: ranges are [a-z].
                        # Escapes inside [] are handled by the backslash.
                        # So if we see \-, it's a literal hyphen.
                        # If we see a-z, it's a range.
                        # We need to check if the character is escaped.
                        # Let's just expand the range if it looks like a range.
                        # If content[k] is '\', then the char is content[k+1].
                        # Then we check if content[k+2] is '-'.
                        # If so, we need to check if content[k+3] is the end.
                        # This is too messy.
                        # Let's simplify: we will just collect characters and ranges.
                        # If we see a range, we expand it.
                        # We assume the input is well-formed.
                        # If we see a range, we expand it.
                        # If we see a literal, we add it.
                        # If we see an escape, we add the escaped char.
                        # Let's do this:
                        # If content[k] == '\\', add content[k+1], k+=2
                        # Else if k+2 < len and content[k+1] == '-' and content[k+2] != ']', then range
                        # Else add content[k], k+=1
                        # But wait, what if the range is [a-\]?
                        # Then content[k+1] is '-', content[k+2] is ']'.
                        # So it's not a range.
                        # What if [a-\z]?
                        # Then content[k+1] is '-', content[k+2] is 'z'.
                        # So it is a range.
                        # What if [a-\-]?
                        # Then content[k+1] is '-', content[k+2] is '-'.
                        # So it is a range from 'a' to '-'.
                        # This seems correct.
                        # But what if the start is escaped? [\-z]
                        # Then content[0] is '\', content[1] is '-'.
                        # Then we add '-'. k becomes 2.
                        # Then content[2] is 'z'.
                        # So we add 'z'.
                        # This works.
                        # What if [a-\-]?
                        # content[0] = 'a', content[1] = '-', content[2] = '\', content[3] = '-'.
                        # We see 'a', then '-', then '\'.
                        # We check if content[1] is '-' and content[2] is not ']'.
                        # content[2] is '\', so it's not ']'.
                        # So we treat it as a range from 'a' to '\'.
                        # But we want a range from 'a' to '-'.
                        # The spec says backslash escapes the next character.
                        # So in [a-\-], the '-' after '\' is a literal hyphen.
                        # So the range is 'a' to '\'.
                        # This is correct according to the spec.
                        # So we just need to handle escapes.
                        # Let's implement this logic.
                        pass
                    k += 1
                # Let's rewrite the class parsing loop
                chars = set()
                k = 0
                negated = False
                if k < len(content) and content[k] == '!':
                    negated = True
                    k += 1
                while k < len(content):
                    if content[k] == '\\':
                        if k + 1 < len(content):
                            chars.add(content[k+1])
                            k += 2
                        else:
                            chars.add('\\')
                            k += 1
                    elif k + 2 < len(content) and content[k+1] == '-' and content[k+2] != ']':
                        # Range
                        start = content[k]
                        end = content[k+2]
                        # Expand range
                        for c in range(ord(start), ord(end) + 1):
                            chars.add(chr(c))
                        k += 3
                    else:
                        chars.add(content[k])
                        k += 1
                
                if negated:
                    # We need to know the universe of characters.
                    # The spec says "a class never matches '/'".
                    # So the universe is all characters except '/'.
                    # But we don't know the full universe.
                    # We can just store the negated set and check later.
                    # Or we can store the positive set and negate it.
                    # But we need to know what characters are allowed.
                    # The allowed characters are all characters except '/'.
                    # So if negated, we match any char except '/' and except the chars in the class.
                    # We can store the class as a set of allowed chars, and a flag for negation.
                    # But we need to know the universe.
                    # Let's store the class as a set of chars, and a flag for negation.
                    # When matching, we check if the char is in the set (if not negated) or not in the set (if negated).
                    # And also check if it is '/'.
                    # So we can store the set of chars and the negation flag.
                    tokens.append(('CLASS', chars, negated))
                else:
                    tokens.append(('CLASS', chars, False))
            else:
                tokens.append(('LITERAL', c))
                i += 1
        return tokens

    def match_tokens(tokens, path):
        # We need to match the whole path.
        # We can use recursion or iteration.
        # Since ** can match multiple segments, we need to handle it carefully.
        # We can split the path into segments.
        # But ** can match zero or more segments.
        # So we can use a recursive function that takes the current token index and current path segment index.
        # But the path is a string, not a list of segments.
        # We can split the path by '/' to get segments.
        # But we need to handle the case where the path is empty or has no segments.
        # Let's split the path into segments.
        # If the path is empty, segments = ['']? Or []?
        # If the path is '/', segments = ['', '']?
        # Let's split by '/' and keep empty strings.
        # e.g. 'a/b' -> ['a', 'b']
        # 'a//b' -> ['a', '', 'b']
        # '/a' -> ['', 'a']
        # 'a/' -> ['a', '']
        # '' -> ['']
        # But the spec says "Paths are separated by '/'".
        # And "a path segment that is exactly '**' matches zero or more whole segments".
        # So we need to split the path into segments.
        # Let's split the path by '/'.
        segments = path.split('/')
        
        # We need to match the tokens against the segments.
        # But the tokens can match multiple segments (for **).
        # We can use a recursive function with memoization.
        # state: (token_index, segment_index)
        # We need to match the whole path, so we must reach the end of tokens and end of segments.
        
        # However, the tokens can match a single character (QMARK, CLASS, LITERAL, ESCAPED) or a segment (STAR, STARSTAR).
        # STAR matches zero or more characters in a segment, but never '/'.
        # STARSTAR matches zero or more whole segments.
        
        # Let's define a function match_segment(token, segment) that returns True if the token matches the segment.
        # But STAR and STARSTAR are special.
        # STAR matches zero or more characters in a segment.
        # STARSTAR matches zero or more whole segments.
        
        # We can use a recursive function that takes (token_index, segment_index).
        # If token_index == len(tokens) and segment_index == len(segments): return True
        # If token_index == len(tokens) or segment_index == len(segments): return False
        
        # But STAR can match zero characters, so it can match an empty segment.
        # STARSTAR can match zero segments.
        
        # Let's implement a recursive function with memoization.
        memo = {}
        
        def solve(ti, si):
            if ti == len(tokens) and si == len(segments):
                return True
            if ti == len(tokens) or si == len(segments):
                return False
            
            key = (ti, si)
            if key in memo:
                return memo[key]
            
            token = tokens[ti]
            seg = segments[si]
            
            if token[0] == 'LITERAL':
                if seg == token[1]:
                    res = solve(ti + 1, si + 1)
                else:
                    res = False
            elif token[0] == 'ESCAPED':
                if seg == token[1]:
                    res = solve(ti + 1, si + 1)
                else:
                    res = False
            elif token[0] == 'QMARK':
                # Matches exactly one character, but never '/'
                # But we are matching against a segment, which is a string.
                # QMARK matches exactly one character in the segment.
                # But the segment can be longer than one character.
                # Wait, the spec says "Paths are separated by '/'".
                # And "QMARK matches exactly one character but never '/'".
                # So QMARK matches one character in the segment.
                # But the segment is a string.
                # So QMARK matches the first character of the segment?
                # No, the pattern is matched against the whole path.
                # The pattern is a sequence of tokens.
                # Each token matches a part of the path.
                # LITERAL, QMARK, CLASS, ESCAPED match a single character.
                # STAR matches zero or more characters in a segment.
                # STARSTAR matches zero or more whole segments.
                # So we