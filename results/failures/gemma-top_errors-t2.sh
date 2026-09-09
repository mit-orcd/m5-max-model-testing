top_errors() {
    local logfile="$1"
    local n="$2"

    if [[ ! -f "$logfile" ]]; then
        return
    fi

    # 1. Extract lines containing 'ERROR: '
    # 2. Extract text after 'ERROR: '
    # 3. Replace runs of digits with '#'
    # 4. Count occurrences
    # 5. Sort by count (desc) then signature (asc)
    # 6. Take top n
    grep 'ERROR: ' "$logfile" | \
    sed 's/.*ERROR: //' | \
    sed 's/[0-9]\+/#/g' | \
    sort | \
    uniq -c | \
    sort -k1,1nr -k2,2d | \
    head -n "$n" | \
    sed 's/^[[:space:]]*//' | \
    sed 's/ / /' | \
    awk '{print $1, $2}' | \
    # The logic above needs careful handling of spaces in signatures.
    # Let's use a more robust approach for the final output.
    awk '{
        count=$1; 
        $1=""; 
        sub(/^ /, "", $0); 
        print count, $0
    }'
}