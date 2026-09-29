top_errors() {
    local logfile="$1"
    local n="$2"

    grep -oP '(?<=^ERROR: ).*' "$logfile" 2>/dev/null | \
    sed 's/[0-9]\+/#/g' | \
    sort | \
    uniq -c | \
    awk '{print $1, $2}' | \
    sort -k1,1nr -k2,2 | \
    head -n "$n"
}