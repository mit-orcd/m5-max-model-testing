import re
from typing import List, Tuple

def _tokenize(s: str) -> List[str]:
    """Split a pattern/path on '/' into segments."""
    if not s:
        return []
    return s.split('/')

def _match_char_class(pat: str, i: int) -> Tuple[str, int]:
    """Parse a bracket expression starting at pat[i] (which is '[').
    Returns (regex_fragment, new_i) where new_i points just after the closing ']'.
    Raises ValueError on malformed input.
    """
    assert pat[i] == '['
    i += 1
    negate = False
    if i < len(pat) and pat[i] in ('!', '^'):
        negate = True
        i += 1
    chars = []
    while i < len(pat) and pat[i] != ']':
        if pat[i] == '\\' and i + 1 < len(pat):
            chars.append(re.escape(pat[i + 1]))
            i += 2
        else:
            chars.append(re.escape(pat[i]))
            i += 1
    if i >= len(pat):
        raise ValueError("Unterminated character class")
    # i now at ']'
    i += 1
    if negate:
        return ('[^' + ''.join(chars) + ']', i)
    else:
        return ('[' + ''.join(chars) + ']', i)

def _compile_segment(seg: str) -> str:
    """Compile a single path segment pattern into a regex string that matches
    exactly one path segment (no '/'). The result is anchored with ^...$."""
    res = ''
    i = 0
    n = len(seg)
    while i < n:
        c = seg[i]
        if c == '\\':
            if i + 1 < n:
                res += re.escape(seg[i + 1])
                i += 2
            else:
                res += re.escape('\\')
                i += 1
        elif c == '?':
            res += '[^/]'
            i += 1
        elif c == '*':
            if i + 1 < n and seg[i + 1] == '*':
                # ** within a segment (not the whole segment) — treat as literal? 
                # Per spec, '**' as a whole segment matches multiple segments.
                # Here it's part of a segment; treat '*' as matching non-slash chars.
                res += '[^/]*'
                i += 1
            else:
                res += '[^/]*'
                i += 1
        elif c == '[':
            frag, i = _match_char_class(seg, i)
            res += frag
        else:
            res += re.escape(c)
            i += 1
    return '^' + res + '$'

def match(pattern: str, path: str) -> bool:
    """Returns True if the glob pattern matches the entire path."""
    if not pattern:
        return path == ''
    if not path:
        # Only match if pattern can match empty
        return _matches_empty(pattern)
    
    pat_segs = _tokenize(pattern)
    path_segs = _tokenize(path)
    
    return _match_segments(pat_segs, path_segs)

def _matches_empty(pattern: str) -> bool:
    """Check if pattern matches an empty path."""
    if not pattern:
        return True
    # Only '**' segments can match empty
    pat_segs = _tokenize(pattern)
    if not pat_segs:
        return True
    # Check if all segments are '**'
    return all(s == '**' for s in pat_segs)

def _match_segments(pat_segs: List[str], path_segs: List[str]) -> bool:
    """Match pattern segments against path segments."""
    # Use dynamic programming / backtracking
    return _match_recursive(pat_segs, 0, path_segs, 0)

def _match_recursive(pat_segs: List[str], pi: int, path_segs: List[str], si: int) -> bool:
    """Recursively match pattern segments starting at pi against path segments starting at si."""
    # Base case: both exhausted
    if pi == len(pat_segs) and si == len(path_segs):
        return True
    if pi == len(pat_segs):
        return False
    if si == len(path_segs):
        # Remaining pattern must all be '**'
        return all(s == '**' for s in pat_segs[pi:])
    
    seg = pat_segs[pi]
    
    if seg == '**':
        # Try matching zero or more path segments
        # Zero segments:
        if _match_recursive(pat_segs, pi + 1, path_segs, si):
            return True
        # One or more segments:
        for k in range(si, len(path_segs)):
            if _match_recursive(pat_segs, pi + 1, path_segs, k + 1):
                return True
        return False
    else:
        # Match exactly one path segment
        if si < len(path_segs):
            regex = _compile_segment(seg)
            if re.match(regex, path_segs[si]):
                return _match_recursive(pat_segs, pi + 1, path_segs, si + 1)
        return False