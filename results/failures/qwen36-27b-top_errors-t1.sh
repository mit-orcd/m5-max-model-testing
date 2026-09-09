top_errors() {
    local logfile="$1"
    local n="$2"

    if [ -z "$logfile" ] || [ -z "$n" ]; then
        return
    fi

    if [ ! -f "$logfile" ]; then
        return
    fi

    grep 'ERROR: ' "$logfile" | \
    sed 's/ERROR: //' | \
    sed 's/[0-9]\+/#/g' | \
    sort | \
    uniq -c | \
    sort -k1,1nr -k2 | \
    head -n "$n" | \
    sed 's/^[[:space:]]*//' | \
    awk '{print $1, substr($0, index($0, $2))}'
}