top_errors() {
    local logfile="$1"
    local n="$2"

    if [[ -z "$logfile" || -z "$n" ]]; then
        return 1
    fi

    # Extract ERROR signatures, replace runs of digits with #, count and sort
    grep -o 'ERROR: .*' "$logfile" 2>/dev/null | \
        sed 's/ERROR: //' | \
        sed 's/[0-9]\+#\?/#/g' | \
        sort | \
        uniq -c | \
        sort -k1,1nr -k2,2 | \
        head -n "$n" | \
        awk '{printf "%s %s\n", $1, $2}'
}