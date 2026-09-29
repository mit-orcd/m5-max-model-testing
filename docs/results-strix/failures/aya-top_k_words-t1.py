import re
from collections import Counter

def top_k_words(text, k):
    words = re.findall(r'\w+', text.lower())
    counter = Counter(words)
    return sorted(counter.most_common(k), key=lambda x: (x[1], x[0]))