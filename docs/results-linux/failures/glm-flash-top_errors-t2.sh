top_errors() {
    local logfile="$1"
    local n="$2"
    
    # Check if n is a valid positive integer
    if ! [[ "$n" =~ ^[0-9]+$ ]] || [[ "$n" -eq 0 ]]; then
        return
    fi

    awk -v n="$n" '
    BEGIN {
        count = 0
        max_lines = n
    }
    
    # Match lines starting with "ERROR: " and capture the rest of the line
    /^ERROR: / {
        match($0, /^ERROR: (.*)$/, arr)
        sig = arr[1]
        
        # Replace runs of digits with #
        gsub(/[0-9]+/, "#", sig)
        
        # Increment count for this signature
        sig_count[sig]++
        sigs[sig_count[sig]] = sig
        count++
    }
    
    END {
        # Iterate from highest count to lowest
        for (i = count; i >= 1; i--) {
            # If we have found enough signatures, stop
            if (i > max_lines) break
            
            sig = sigs[i]
            if (sig != "") {
                print i, sig
            }
        }
    }
    ' "$logfile"
}