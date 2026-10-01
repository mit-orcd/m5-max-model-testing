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
        for (s in sigs) {
            sigs[s] = count - sigs[s] " " s;
        }
        
        # Print top n
        for (i = 0; i < n; i++) {
            max = -1;
            for (s in sigs) {
                if (sigs[s] > max) {
                    max = sigs[s];
                    best = s;
                }
            }
            if (max != -1) {
                split(max, parts, " ");
                print parts[2] " " parts[1];
                delete sigs[best];
            }
        }
    }
    ' "$logfile"
}