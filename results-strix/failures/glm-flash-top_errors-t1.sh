top_errors() {
    local logfile="$1"
    local n="$2"
    
    # Extract lines starting with 'ERROR:', remove the prefix, and replace digits with #
    awk '/^ERROR:/ {
        sub(/^ERROR: /, "")
        gsub(/[0-9]+/, "#")
        print
    }' "$logfile" | sort | uniq -c | sort -rnk1,1 -k2,2 | head -n "$n"
}