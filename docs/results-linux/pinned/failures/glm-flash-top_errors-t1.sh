top_errors() {
    local logfile="$1"
    local n="$2"
    
    awk -v n="$n" '
    BEGIN { count = 0; }
    /ERROR: / {
        # Extract text after "ERROR: " and replace digits with #
        sig = substr($0, index($0, "ERROR: ") + 7);
        gsub(/[0-9]+/, "#", sig);
        
        # Store signature and count
        sigs[count] = sig;
        counts[count] = 1;
        
        # Update count
        count++;
    }
    END {
        # Sort by count descending, then signature ascending
        for (i = 0; i < count; i++) {
            for (j = i + 1; j < count; j++) {
                if (counts[i] < counts[j] || (counts[i] == counts[j] && sigs[i] > sigs[j])) {
                    # Swap
                    temp = sigs[i];
                    sigs[i] = sigs[j];
                    sigs[j] = temp;
                    
                    temp = counts[i];
                    counts[i] = counts[j];
                    counts[j] = temp;
                }
            }
        }
        
        # Print top n
        for (i = 0; i < count && i < n; i++) {
            print counts[i] " " sigs[i];
        }
    }
    ' "$logfile"
}