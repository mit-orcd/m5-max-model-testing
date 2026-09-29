top_errors() {
    local logfile="$1"
    local n="$2"

    # Extract ERROR lines, get text after 'ERROR: ', replace runs of digits with #
    # Then count, sort by count desc then signature asc, take top n
    grep -E 'ERROR: ' "$logfile" 2>/dev/null | \
    sed -E 's/.*ERROR: //' | \
    sed -E 's/[0-9]+/#/g' | \
    sort | \
    uniq -c | \
    awk '{print $1, $2}' | \
    sort -k1,1nr -k2,2 | \
    head -n "$n"
}