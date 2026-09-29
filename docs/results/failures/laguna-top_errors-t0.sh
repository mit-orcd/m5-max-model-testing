top_errors() {
    local logfile="$1"
    local n="$2"
    
    grep 'ERROR: ' "$logfile" 2>/dev/null | \
        sed 's/ERROR: //' | \
        sed 's/[0-9]\+/#/g' | \
        sort | uniq -c | \
        sort -k1,1nr -k2,2 | \
        head -n "$n" | \
        awk '{print $1, substr($0, index($0,$2))}'
}