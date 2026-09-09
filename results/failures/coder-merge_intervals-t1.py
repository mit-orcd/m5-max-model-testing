def merge_intervals(intervals):
    if not intervals:
        return []
    
    # Sort intervals by start time
    sorted_intervals = sorted(intervals, key=lambda x: x[0])
    
    merged = [sorted_intervals[0]]
    
    for current in sortedials[1:]:
        last = merged[-1]
        if current[0] <= last[1]:
            # Merge overlapping intervals
            merged[-1] = [last[0], max(last[1], current[1])]
        else:
            # No overlap, add current interval
            merged.append(current)
    
    return merged