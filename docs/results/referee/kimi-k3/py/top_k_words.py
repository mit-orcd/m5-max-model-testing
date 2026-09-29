import re
from collections import Counter


def top_k_words(text, k):
    words = re.findall(r"[A-Za-z]+", text.lower())
    counts = Counter(words)
    best = sorted(counts.items(), key=lambda kv: (-kv[1], kv[0]))[:k]
    return best
