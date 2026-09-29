top_errors() {
    local logfile="$1"
    local n="$2"

    awk -v n="$n" '
    BEGIN { count = 0 }

    # Match lines starting with "ERROR: " to extract the signature
    /^ERROR: / {
        # Extract text after "ERROR: " (everything from index 8 onwards)
        sig = substr($0, 9)

        # Replace sequences of digits with '#'
        while (match(sig, /[0-9]+/)) {
            start = RSTART
            len = RLENGTH
            sub(/[0-9]+/, "#", sig)
        }

        # Store the cleaned signature and increment count
        sigs[sig]++
        count++
    }

    END {
        if (count == 0) exit

        # Create an array of signatures, then sort
        for (s in sigs) {
            sig_array[count++] = s
        }
        count--

        # Sort the array: first by count (descending), then by signature (ascending)
        for (i = 1; i <= count; i++) {
            for (j = i + 1; j <= count; j++) {
                if (sigs[sig_array[i]] < sigs[sig_array[j]] || 
                    (sigs[sig_array[i]] == sigs[sig_array[j]] && sig_array[i] > sig_array[j])) {
                    temp = sig_array[i]
                    sig_array[i] = sig_array[j]
                    sig_array[j] = temp
                }
            }
        }

        # Print the top n entries
        for (i = 1; i <= n && i <= count; i++) {
            print sigs[sig_array[i]] " " sig_array[i]
        }
    }
    ' "$logfile"
}