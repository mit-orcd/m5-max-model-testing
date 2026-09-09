def run_length_encode(s):
    if not s:
        return []
    
    result = []
    current_char = s[0]
    count = 1
    
    for i inimport re
from itertools import groupby

def run_length_encode(s):
    if not s:
        return []
    
    groups = []
    for char, group in groupby(s):
        count = len(list(group))
        groups.append((char, count))
    
    return groups