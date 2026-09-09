import re
from collections import Counter

def top_k_words(text, k):
    words = re.findall(r'[a-zA-Z]+', text.lower())
    counts = Counter(words)
    return sorted(counts.items(), key=lambda x: (-x[1], x[num]))