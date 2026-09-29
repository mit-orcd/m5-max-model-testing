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
        
        # Increment count
        count++
    }
    END {
        # Sort by count descending, then signature ascending
        for (i = 0; i < count; i++) {
            for (j = i + 1; j < count; j++) {
                if (counts[i] < counts[j] || (counts[i] == counts[j] && sigs[i] > sigs[j])) {
                    # Swap
                    temp_sig = sigs[i]
                    temp_cnt = counts[i]
                    sigs[i] = sigs[j]
                    counts[i] = counts[j]
                    sigs[j] = temp_sig
                    counts[j] = temp_cnt
                }
            }
        }
        
        # Print top n
        for (i = 0; i < count && i < n; i++) {
            print counts[i] " " sigs[i]
        }
    }
    ' "$logfile"
}