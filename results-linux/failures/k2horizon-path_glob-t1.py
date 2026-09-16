import re

def match(pattern, path):
    tokens = _tokenize(pattern)
    return _match_tokens(tokens, path.split('/'))

def _tokenize(pattern):
    tokens = []
    i = 0
    n = len(pattern)
    while i < n:
        c = pattern[i]
        if c == '\\':
            if i + 1 < n:
                tokens.append(Escaped(pattern[i + 1]))
                i += 2
            else:
                tokens.append(Escaped(c))
                i += 1
        elif c == '[':
            j = i + 1
            negated = False
            if j < n and pattern[j] == '!':
                negated = True
                j += 1
            chars = []
            while j < n and pattern[j] != ']':
                if j + 2 < n and pattern[j + 1] == '-':
                    lo = pattern[j]
                    hi = pattern[j + 2]
                    for code in range(ord(lo), ord(hi) + 1):
                        chars.append(chr(code))
                    j += 3
                else:
                    chars.append(pattern[j])
                    j += 1
            if j >= n:
                # Unclosed bracket: treat '[' literally
                tokens.append(Escaped('['))
                i += 1
            else:
                if negated:
                    tokens.append(CharClass(chars, negated=True))
                else:
                    tokens.append(CharClass(chars, negated=False))
                i = j + 1
        elif c == '*':
            if i + 2 < n and pattern[i + 1] == '*' and pattern[i + 2] == '/':
                tokens.append(DoubleStar())
                i += 3
            else:
                tokens.append(Star())
                i += 1
        elif c == '?':
            tokens.append(Question())
            i += 1
        else:
            tokens.append(Escaped(c))
            i += 1
    return tokens

class Escaped:
    __slots__ = ('c',)
    def __init__(self, c):
        self.c = c

class Star:
    __slots__ = ()
    def __repr__(self):
        return 'Star()'

class DoubleStar:
    __slots__ = ()
    def __repr__(self):
        return 'DoubleStar()'

class Question:
    __slots__ = ()
    def __repr__(self):
        return 'Question()'

class CharClass:
    __slots__ = ('chars', 'negated')
    def __init__(self, chars, negated=False):
        self.chars = frozenset(chars)
        self.negated = negated
    def matches(self, ch):
        if self.negated:
            return ch not in self.chars
        return ch in self.chars