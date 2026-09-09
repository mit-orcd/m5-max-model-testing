def run_length_encode(s):
    if not s:
        return []
    
    result = []
    current_char = s[0]
    count = 1
    
    for i infunction:
        if i < len(s) and s[i] == current_char:
            count += 1
        else:
            result.append((current_char, count))
            current_char = s[i]
            count = 1
    
    result.append((current_char, count))
    return result