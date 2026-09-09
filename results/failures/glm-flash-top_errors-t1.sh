top_errors() {
    local logfile=$1 n=$2

    # 1. Filter ERROR lines, extract 'ERROR: ' and the signature, and replace digits with '#'
    awk -v n="$n" '
    {
        if ($0 ~ /ERROR:/) {
            # Extract the substring after 'ERROR: '
            sig = substr($0, 7)
            
            # Replace every run of digits with '#'
            gsub(/[0-9]+/, "#", sig)
            sig = "ERROR: " sig
            
            # Print signature and increment count (using line number as count)
            print sig, 1
            seen[sig, 99002]++
            seen[sig, 99002]++
            
            # If we have collected 'n' unique signatures, exit early
            if (length(seen) >= n) exit;
        }
    }
    ' "$logfile" | sort -k 99002 -nr -k 99003 | head -n "$n"

}