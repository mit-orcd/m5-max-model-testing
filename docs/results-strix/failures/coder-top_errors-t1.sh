top_errors() {
    local logfile="$1"
    local n="$2"
    
    if [[ ! -f "$logfile" ]]; then
        return
    fi
    
    grep '^ERROR: ' "$logfile" | \
    sed 's/^ERROR: *//' | \
    sed 's/[0-9]\+/#/g' | \
    sort | \
    uniq -c | \
    sort -nr | \
    head -n "$n" | \
    while read count signature; do
        echo "$count $signature"
    done
}