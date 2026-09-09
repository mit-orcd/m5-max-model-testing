import re2

def top_k_words(text, k):
    # Use re2 for efficient regex matching, falling back to re if not available.
    try:
        import re
    except ImportError:
        import re

    # 1. Extract words: maximal runs of ASCII letters, case-insensitive.
    # re.findall returns a list of all non-overlapping matches.
    # re.IGNORECASE handles case-insensitivity.
    words = re.findall(r'[A-Za-z]+', text, re.IGNORECASE)

    # 2. Count frequencies.
    # We use a dictionary to store word counts.
    # We lowercase the words as required.
    counts = {}
    for word in words:
        word_lower = word.lower()
        counts[word_lower] = counts.get(word_lower, 0) + 1

    # 3. Sort the words.
    # We need to sort by:
    # - Frequency (count) in descending order.
    # - Alphabetical order (word) in ascending order for ties.
    
    # sorted() returns a new list.
    # The key function returns a tuple ( -count, word ).
    # -count sorts by count in descending order.
    # word sorts by word in ascending order.
    sorted_words = sorted(counts.keys(), key=lambda w: (-counts[w], w))

    # 4. Get the k most frequent words.
    # We take the first k elements from the sorted list.
    # If k is larger than the number of unique words, it will just return all of them.
    top_k = sorted_words[:k]

    # 5. Return a list of (word, count) tuples.
    # We use a list comprehension to create the final list.
    return [(word, counts[word]) for word in top_k]