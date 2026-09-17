import re
from collections import Counter

def top_k_words(text, k):
    # Extract words: sequences of ASCII letters, case-insensitive
    words = re.findall(r'[A-Za-z]+', text)
    
    # Count frequencies
    counts = Counter(words)
    
    # Sort by frequency (descending), then alphabetically (ascending)
    sorted_words = sorted(counts.items(), key=lambda x: (-x[1], x[0]))
    
    # Return the top k entries
    return sorted_words[:k]