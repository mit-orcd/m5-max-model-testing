top_errors() {
    local logfile="$1"
    local n="$2"

    [ ! -f "$logfile" ] && return

    sed -n 's/.*ERROR: \(.*\)/\1/p' "$logfile" | \
    sed -E 's/[0-9]+/#/g' | \
    sort | \
    uniq -c | \
    sort -k1,1nr -k2,2d | \
    head -n "$n" | \
    awk '{print $1, $2}' | \
    sed 's/ / /' | \
    while read -r count sig; do
        echo "$count $sig"
    done
}