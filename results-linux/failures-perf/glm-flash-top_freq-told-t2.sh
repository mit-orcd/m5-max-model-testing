top_freq() {
    local file="$1"
    
    # Sort the file to group identical values together. 
    # 'n' sorts numerically, which handles leading zeros correctly.
    # 's' sorts strings, preserving the original format.
    # 'k1' sorts based on the first field (the value).
    local sorted_file
    sorted_file=$(sort -n -s -k1 "$file")
    
    # Iterate through the sorted list, counting consecutive occurrences.
    # 'uniq -c' prints the count followed by the line.
    # 'awk' performs the logic to accumulate counts and track the top 10.
    printf '%s\n' "$sorted_file" | awk '
    {
        # If the current value is the same as the previous one, increment the count.
        if (prev == $1) {
            cnt++;
        } else {
            # If it's a new value, first check if the previous value should be kept.
            # We maintain a list of top 10 values in an array.
            # We check if the new value's count is greater than the smallest 
            # count currently in the top 10.
            
            # First, add the previous value to the top 10 list if it has a count > 0.
            if (prev != "") {
                # Add the previous value to the array
                top[prev] = cnt;
            }

            # Now, check if the new value should be in the top 10.
            # We need to check if its count is greater than the smallest count 
            # in the top 10 list.
            # To do this efficiently, we maintain a second array, 'min_count', 
            # which stores the smallest count for each value in the top 10.
            
            # We don't know the smallest count yet, so we initialize min_count 
            # with a very large number.
            min_cnt = 999999999;
            
            # Iterate through the top 10 values to find the smallest count.
            for (i = 0; i < 10; i++) {
                if (top[i] != "" && top[i] < min_cnt) {
                    min_cnt = top[i];
                }
            }
            
            # If the current value's count is greater than the smallest count 
            # in the top 10, then it should replace the one with the smallest count.
            if ($2 > min_cnt) {
                # Find the value in the top 10 that has the smallest count 
                # and replace it with the current value.
                for (i = 0; i < 10; i++) {
                    if (top[i] == min_cnt) {
                        top[i] = $2;
                        break;
                    }
                }
            }
            
            # Reset for the next value.
            prev = $1;
            cnt = $2;
        }
    }
    
    # After the loop, we need to add the last value to the top 10 list.
    # The same logic as above applies.
    if (prev != "") {
        top[prev] = cnt;
    }
    
    # Now, we need to sort the top 10 values by their counts in descending order.
    # We'll use an associative array to store the values and their counts.
    
    # Create a list of all values in the top 10 array.
    for (val in top) {
        list[list_idx++] = val;
    }
    
    # Sort the list of values based on their counts in descending order.
    # We'll use a simple bubble sort for this.
    for (i = 0; i < list_idx; i++) {
        for (j = i + 1; j < list_idx; j++) {
            if (top[list[i]] < top[list[j]]) {
                # Swap
                temp = list[i];
                list[i] = list[j];
                list[j] = temp;
            }
        }
    }
    
    # Print the top 10 values.
    for (i = 0; i < 10; i++) {
        if (list[i] != "") {
            printf("%d %s\n", top[list[i]], list[i]);
        }
    }
    '
}