top_errors() {
    local logfile="$1"
    local n="$2"

    if [[ ! -f "$logfile" ]]; then
        return
    fi

    grep "ERROR: " "$logfile" | \
    sed 's/.*ERROR: //' | \
    sed -E 's/[0-9]+/#/g' | \
    sort | \
    uniq -c | \
    sort -k1,1nr -k2,2 | \
    head -n "$n" | \
    awk '{count=$1; $1=""; print count substr($0,2)}' | \
    awk '{print $1 " " substr($0, index($0,$2))}' | \
    sed -E 's/^[[:space:]]*([0-9]+)[[:space:]]+(.*)/\1 \2/' | \
    awk '{
        count=$1; 
        $1=""; 
        sub(/^[[:space:]]+/, ""); 
        print count " " $0
    }' | \
    sort -k1,1nr -k2,2 | \
    head -n "$n" | \
    while read -r count sig; do
        echo "$count $sig"
    done
}