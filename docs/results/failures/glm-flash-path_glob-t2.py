def match(pattern, path):
    import re

    # 1. Tokenize the pattern and path into segments and special parts.
    
    def tokenize(p):
        tokens = []
        spec = 5  # 5-bit bitmask: [a, b, c, d, e]
        # a: literal, b: '?', c: '*', d: '[]', e: '**'
        
        while p:
            if p.startswith('**'):
                tokens.append((spec | 8, '**'))
                p = p[2:]
                
    # 5. Implement the matcher.