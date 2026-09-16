import re

def match(pattern, path):
    # Compile the regex pattern
    regex = _to_regex(pattern)
    # Ensure the match covers the entire path
    return bool(regex.fullmatch(path))

def _to_regex(pattern):
    # Escape special regex characters that are not glob-specific
    # We must escape: . ^ $ + { } [ ] \ | ( )
    # Note: * ? [ ] are handled by the logic below, but we need to be careful with regex meta-characters
    # that are not glob metacharacters.
    
    # We'll build the pattern character by character
    regex_parts = []
    
    i = 0
    n = len(pattern)
    
    while i < n:
        char = pattern[i]
        
        if char == '\\':
            if i + 1 < n:
                # Escaped character, treat as literal
                # We need to escape it for regex as well to prevent interpretation
                regex_parts.append(re.escape(pattern[i+1]))
                i += 2
            else:
                # Trailing backslash, treat as literal backslash
                regex_parts.append(re.escape('\\'))
                i += 1
                
        elif char == '?':
            # Matches exactly one character, not '/'
            regex_parts.append('[^/]')
            i += 1
            
        elif char == '*':
            # Matches zero or more characters, but never '/'
            # This is equivalent to '[^/]*'
            regex_parts.append('[^/]*')
            i += 1
            
        elif char == '[':
            # Handle character classes: [abc], [a-z], [!abc]
            # Find the closing bracket
            j = i + 1
            while j < n and pattern[j] != ']':
                j += 1
            
            if j == n:
                # Unclosed class, treat as literal '['
                regex_parts.append(re.escape('['))
                i += 1
            else:
                class_part = pattern[i+1:j]
                # Handle negation: [!abc] -> [^abc]
                if class_part.startswith('!'):
                    class_part = '^' + class_part[1:]
                
                # Convert the class to a regex set
                # We need to handle ranges like a-z
                # We'll process the class string to build the regex set
                class_regex_parts = []
                k = 0
                m = len(class_part)
                
                while k < m:
                    c1 = class_part[k]
                    if k + 2 < m and class_part[k+1] == '-' and class_part[k+2] != ']':
                        # It's a range like a-z
                        c2 = class_part[k+2]
                        class_regex_parts.append(f'{c1}-{c2}')
                        k += 3
                    else:
                        # It's a single character
                        # We need to escape it for regex
                        class_regex_parts.append(re.escape(c1))
                        k += 1
                
                class_regex = ''.join(class_regex_parts)
                # If the class is empty (e.g. []), match nothing
                if not class_regex:
                    class_regex = '(?!)'
                
                regex_parts.append(f'[{class_regex}]')
                i = j + 1
                
        else:
            # Literal character
            regex_parts.append(re.escape(char))
            i += 1
            
    return '^' + ''.join(regex_parts) + '$'