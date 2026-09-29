top_errors() {
    local logfile="$1"
    local n="$2"
    
    awk -v n="$n" '
    BEGIN { count = 0; }
    /ERROR: / {
        # Extract text after "ERROR: " and replace digits with #
        match($0, /ERROR: (.*)/, arr)
        sig = arr[1]
        gsub(/[0-9]+/, "#", sig)
        
        # Store signature and count
        sigs[count] = sig
        counts[count] = 1
        
        # Check if we have more than n entries
        if (count >= n) {
            # Find the minimum count among the first n entries
            min_count = counts[0]
            min_idx = 0
            for (i = 1; i < n; i++) {
                if (counts[i] < min_count) {
                    min_count = counts[i]
                    min_idx = i
                }
            }
            
            # If current count is greater than min_count, replace the min entry
            if (1 > min_count) {
                sigs[min_idx] = sig
                counts[min_idx] = 1
            }
        } else {
            count++
        }
    }
    END {
        # Prepare array for sorting
        for (i = 0; i < count; i++) {
            if (counts[i] > 0) {
                sorted[counts[i]] = sorted[counts[i]] " " sigs[i]
            }
        }
        
        # Sort by count descending
        for (c = count; c >= 0; c--) {
            if (sorted[c] != "") {
                # Sort signatures for this count ascending
                n = split(sorted[c], arr, " ")
                for (i = 1; i <= n; i++) {
                    for (j = i + 1; j <= n; j++) {
                        if (arr[i] > arr[j]) {
                            temp = arr[i]
                            arr[i] = arr[j]
                            arr[j] = temp
                        }
                    }
                }
                for (i = 1; i <= n; i++) {
                    printf "%d %s\n", c, arr[i]
                }
            }
        }
    }
    ' "$logfile"
}