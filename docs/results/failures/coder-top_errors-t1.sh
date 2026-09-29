top_errors() {
    local logfile="$1"
    local n="$2"
    
    if [[ ! -r "$logfile" ]]; then
        return
    fi
    
    grep -E 'ERROR:.*' "$logfile" | \
    sed 's/ERROR: *//' | \
    sed 's/#[#]*/#/g' | \
    sed 's/#[#]*/#/g' | \
    sed 's/[0-9]/#/g' | \
    sort | \
    uniq -c | \
    sort -nr -k1,1 -k2,2 | \
    head -n "$n" | \
    awk '{printf "%d %s\n", $1, $2}'
}