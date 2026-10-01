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
        sigs[sig] = sigs[sig] + 1;
        count++;
    }
    END {
        # Sort signatures by count descending, then signature ascending
        for (sig in sigs) {
            printf "%s %d\n", sig, sigs[sig];
        } 
        # Sort the array
        asort(sigs, sorted_sigs);
        
        # Print top n results
        for (i = count; i > count - n && i > 0; i--) {
            print sorted_sigs[i] " " sigs[sorted_sigs[i]];
        }
    }
    ' "$logfile"
}