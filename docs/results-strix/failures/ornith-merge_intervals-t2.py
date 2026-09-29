def merge_intervals(intervals):
    if not intervals:
        return []
    
    intervals = sorted(intervals)
    merged = [intervals[0][:]]
    
    for start, end in intervals[1:]:
        last_start, last_end = merged[-1]
        if start <= last_end + 1:
            merged[-1][1] = max(last_end, end)
        else:
            merged.append([start, end])
    
    return merged