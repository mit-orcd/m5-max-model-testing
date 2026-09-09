def run_length_encode(s):
    i, result, count = 0, [], 1
    while i < len(s):
        result.append((s[i], count))
        i += count
        if i < len(s):
            count = 1
        else:
            count += 1
    return result