top_errors() {
    local logfile=$1 n=$2
    [ -f "$logfile" ] || return 0
    grep -o 'ERROR: [^[:space:]][^[:space:]]*' "$logfile" 2>/dev/null | \
    sed -E 's/^ERROR: //; s/[0-9]+/#/g' | \
    sort | uniq -c | sort -k1,1nr -k2,2 | head -n "$n" | \
    awk '{print $1, substr($0, index($0, $2))}'
}