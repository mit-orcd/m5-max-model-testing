import re
from collections import Counter

def top_k_words(text, k):
    words = re.findall(r'[a-zA-Z]+', text.lower())
    word_counts = Counter(words)
    return sorted(word_counts.items(), key=lambda x: (-x[1], x[def top_k_words(text, k):
    words = re.findall(r'[a-zA-Z]+', text.lower())
    word_counts = Counter(words)
    return sorted(word_counts.items(), key=lambda x: (-x[1], x[0]))