top_errors() {
    local logfile="$1"
    local n="$2"
    
    if [[ ! -r "$logfile" ]]; then
        return
    fi
    
    grep -E 'ERROR:.*' "$logfile" | \
    sed 's/ERROR: *//' | \
    sed 's/#[#0-9]*/#/g' | \
    sort | \
    uniq -c | \
    sort -rn | \
    head -n "$n" | \
    awk '{print $1 " " $2}'
}