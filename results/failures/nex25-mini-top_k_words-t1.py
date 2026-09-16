import re
from collections import Counter


def top_k_words(text, k):
    if k <= 0:
        return []
    return Counter(re.findall(r"[A-Za-z]+", text)).most_common(k)