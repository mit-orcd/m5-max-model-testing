from collections import Counter
import re


def top_k_words(text, k):
    words = re.findall(r'\w+', text.lower())
    counts = Counter(words)
    return sorted(counts.most_common(k), key=lambda x: (x[1], x[0]))