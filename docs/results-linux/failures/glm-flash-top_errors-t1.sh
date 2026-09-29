top_errors() {
    local logfile="$1"
    local n="$2"
    
    awk -v n="$n" '
    /ERROR: / {
        # Extract the text after 'ERROR: ' and remove trailing newline
        msg = substr($0, index($0, "ERROR: ") + 7)
        
        # Replace runs of digits with #
        gsub(/[0-9]+/, "#", msg)
        
        # Create signature key: original message
        key = msg
        
        # Count occurrences
        count[key]++
    }
    END {
        # Convert counts to array and sort
        for (key in count) {
            arr[count[key], key] = key
        }
        
        # Sort by count descending, then key ascending
        n = asort(arr, sorted_arr, "@val_num_desc")
        
        # Print top n entries
        for (i = 1; i <= n && i <= length(sorted_arr); i += 2) {
            printf "%d %s\n", sorted_arr[i], sorted_arr[i+1]
        }
    }
    ' "$logfile"
}