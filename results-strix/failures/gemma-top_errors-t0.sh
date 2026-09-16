top_errors() {
    local logfile="$1"
    local n="$2"

    grep "ERROR: " "$logfile" | \
    sed 's/.*ERROR: //' | \
    sed -E 's/[0-9]+/#/g' | \
    sort | \
    uniq -c | \
    sort -k1,1nr -k2,2 | \
    head -n "$n" | \
    awk '{count=$1; $1=""; print count substr($0,2)}'
}