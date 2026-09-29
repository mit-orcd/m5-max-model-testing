top_errors() {
    local logfile="$1"
    local n="$2"

    if [[ -z "$logfile" || -z "$n" ]]; then
        return 1
    fi

    if [[ ! -f "$logfile" ]]; then
        return 1
    fi

    # Extract ERROR lines, get the part after "ERROR: ", replace digits with #, count, sort, and print top n
    grep 'ERROR: ' "$logfile" 2>/dev/null | \
        sed 's/.*ERROR: //' | \
        sed 's/[0-9]*/#/g' | \
        sort | \
        uniq -c | \
        sort -k1,1nr -k2,2 | \
        head -n "$n" | \
        awk '{print $1, $2}'
}