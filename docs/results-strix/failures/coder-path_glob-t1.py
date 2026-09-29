def match(pattern, path):
    def is_match(p, s, pi, si):
        while pi < len(p) and si < len(s):
            if p[pi] == '*':
                # Skip consecutive stars
                while pi + 1 < len(p) and p[pi + 1] == '*':
                    pi += 1
                # Try all possible matches for the star
                pi += 1
                # If star is at the end, it matches everything
                if pi >= len(p):
                    return True
                # Try matching different parts of the string
                while si < len(s):
                    if is_match(p, s, pi, si):
                        return True
                    if s[si] == '/':
                        break
                    si += 1
                return False
            elif p[pi] == '?':
                if s[si] == '/':
                    return False
                pi += 1
                si += 1
            elif p[pi] == '[':
                # Parse character class
                pi += 1
                neg = False
                if pi < len(p) and p[pi] == '!':
                    neg = True
                    pi += 1
                if pi >= len(p) or p[pi] == ']':
                    return False
                match_char = False
                while pi < len(p) and p[pi] != ']':
                    if pi + 2 < len(p) and p[pi + 1] == '-' and p[pi + 2] != ']':
                        # Range
                        if p[pi] <= s[si] <= p[pi + 2]:
                            match_char = True
                            break
                        pi += 1
                    elif p[pi] == s[si]:
                        match_char = True
                        break
                    pi += 1
                if pi >= len(p) or p[pi] != ']':
                    return False
                if (neg and not match_char) or (not neg and match_char):
                    pi += 1
                    si += 1
                else:
                    return False
            elif p[pi] == '\\':
                pi += 1
                if pi >= len(p):
                    return False
                if p[pi] != s[si]:
                    return False
                pi += 1
                si += 1
            else:
                if p[pi] != s[si]:
                    return False
                pi += 1
                si += 1
        # Handle remaining pattern
        while pi < len(p) and p[pi] == '*':
            pi += 1
        # If we've consumed the whole pattern, check if we've consumed the whole string
        return pi >= len(p) and si >= len(s)
    
    # Handle the ** case specially
    # Convert ** to a pattern that can match zero or more segments
    if '**' in pattern:
        # Replace ** with a special marker that we can handle
        parts = pattern.split('**')
        # Build a new pattern that handles the ** behavior
        new_pattern = parts[0]
        for i in range(1, len(parts)):
            if i == 1 and len(parts[0]) > 0 and parts[0][-1] != '/':
                new_pattern += '.*'
            elif i == len(parts) - 1 and len(parts[-1]) > 0 and parts[-1][0] != '/':
                new_pattern += '.*'
            else:
                new_pattern += '.*'
            new_pattern += parts[i]
        pattern = new_pattern
    
    # Split path into segments
    segments = path.split('/')
    # Convert path to pattern with segments
    path_pattern = '/'.join(['*'] * len(segments)) if segments else '*'
    
    # Handle the ** case more carefully
    def clean_pattern(p):
        # Remove duplicate stars and handle ** specially
        result = []
        i = 0
        while i < len(p):
            if i + 1 < len(p) and p[i] == '*' and p[i+1] == '*':
                # Handle **
                if i > 0 and p[i-1] != '/':
                    result.append('*')
                else:
                    # ** can match 0 or more segments, so we'll use a special approach
                    # This is a simplified version that works for the basic case
                    result.append('*')
                # Skip all consecutive **s
                while i < len(p) and p[i] == '*':
                    i += 1
                continue
            result.append(p[i])
            i += 1
        return ''.join(result)
    
    # This is a simplified implementation that works for basic cases
    # A full implementation would be quite complex due to the complexity of ** matching
    return is_match(pattern, path, 0, 0)