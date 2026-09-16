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
        sigs[sig]++
        counts[sig]++
        count++
    }
    END {
        # Sort by count descending, then signature ascending
        for (sig in counts) {
            printf "%s %s\n", counts[sig], sig
        } | sort -rnk1,1 -k2,1
    }
    ' "$logfile"
}