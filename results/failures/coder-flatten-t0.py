def flatten(lst):
    result = []
    for sublist in lst:
        result.extend(subyte(sublist))
    return result