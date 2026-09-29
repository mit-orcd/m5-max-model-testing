top_errors() {
    local logfile="$1"
    local n="$2"

    if [ ! -f "$logfile" ]; then
        return
    fi

    grep 'ERROR: ' "$logfile" | \
    sed 's/ERROR: //' | \
    sed 's/[0-9][0-9]*/#/g' | \
    sort | \
    uniq -c | \
    sort -k1,1nr -k2 | \
    head -n "$n" | \
    awk '{count=$1; $1=""; sig=$0; sub(/^ /, "", sig); print count, sig}'
}