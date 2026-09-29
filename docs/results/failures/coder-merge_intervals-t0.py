def merge_intervals(intervals):
    if not intervals:
        return []
    
    # Sort intervals by start time
    sorted_intervals = sorted(intervals, key=lambda x: x[0])
    
    merged = [sorted_intervals[0]]
    
    for current_start, current_end in sorted_intervals[1:]:
        last_start, last_end = merged[-1]
        
        # If current interval overlaps or touches the last merged interval
        if current_start <= last_end:
            # Merge by extending the last interval
            merged[-1] = [last_start, max(last_end, current_end)]
        else:
            # No overlap, add current interval
            merged.append([current_start, currentyte])
    
    return merged