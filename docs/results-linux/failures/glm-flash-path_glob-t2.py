import re

def match(pattern, path):
    """
    Determines if a glob pattern matches a whole path.
    """
    # 1. Handle Escape Sequences
    # Replace backslash-escaped characters with a safe placeholder
    # This prevents regex special chars inside pattern strings from being interpreted
    # by the regex engine later.
    pattern = re.sub(r'\\(.)', r'\\\1', pattern)

    # 2. Prepare Regex Construction
    # We build a regex string from the glob pattern.
    # We use a raw string (r'...') for the regex parts to avoid double escaping issues.
    regex_parts = []

    # Track if we are currently inside a character class
    in_class = False
    # Track if the character class is negated
    negated = False

    i = 0
    n = len(pattern)

    while i < n:
        char = pattern[i]

        if char == '\\':
            # Escape sequence: next char is literal
            # Ensure we don't go out of bounds
            if i + 1 < n:
                regex_parts.append(re.escape(pattern[i + 1]))
                i += 2
            else:
                # Trailing backslash, treat as literal backslash
                regex_parts.append('\\')
                i += 1
            continue

        if char == '[':
            in_class = True
            negated = False
            regex_parts.append('[')
            i += 1
            continue

        if char == ']':
            if in_class:
                regex_parts.append(']')
                in_class = False
                negated = False
            else:
                # Unmatched ']' in glob pattern treated as literal
                regex_parts.append('\\]')
            i += 1
            continue

        if char == '!':
            if in_class:
                negated = True
                regex_parts.append('^')
            else:
                # '!' outside class is literal
                regex_parts.append('\\!')
            i += 1
            continue

        if char == '*':
            # '*' matches zero or more characters, excluding '/'.
            # Equivalent to [^/]* in regex.
            regex_parts.append('[^/]*')
            i += 1
            continue

        if char == '?':
            # '?' matches exactly one character, excluding '/'.
            # Equivalent to [^/] in regex.
            regex_parts.append('[^/]')
            i += 1
            continue

        if char == '/':
            # Path separator
            regex_parts.append('/')
            i += 1
            continue

        # Default: Literal character
        regex_parts.append(re.escape(char))
        i += 1

    # Handle case where pattern ends inside a class
    if in_class:
        regex_parts.append(']')

    # 3. Construct and Compile Regex
    # Join parts. Note: re.escape handles literal dots and stars, 
    # but we manually handled * and ? to enforce the '/' rule.
    # We use DOTALL to allow .* to match across lines if needed, 
    # though our glob implementation restricts '/' specifically.
    regex_str = ''.join(regex_parts)
    
    # The regex must match the ENTIRE string (start to end).
    # $ ensures the path is fully consumed.
    regex_str = f'^{regex_str}$'
    
    try:
        prog = re.compile(regex_str)
    except re.error:
        return False

    # 4. Execute Match
    return prog.match(path) is not None