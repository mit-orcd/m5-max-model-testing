top_errors() {
    local logfile="$1"
    local n="$2"

    # Extract ERROR lines, get text after 'ERROR: ', normalize digits to #, count, sort, head
    grep -h '^ERROR: ' "$logfile" 2>/dev/null | \
        sed 's/^ERROR: //' | \
        sed 's/[0-9]\+/#/g' | \
        sort | \
        uniq -c | \
        awk '{print $1, $2}' | \
        sort -k1,1nr -k2,2 | \
        head -n "$n"
}