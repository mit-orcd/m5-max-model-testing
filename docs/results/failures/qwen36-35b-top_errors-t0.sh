top_errors() {
    local logfile="$1"
    local n="$2"

    if [[ ! -f "$logfile" ]]; then
        return
    fi

    grep -oP 'ERROR: \K.*' "$logfile" 2>/dev/null | \
        sed 's/[0-9]\+#/##/g; s/[0-9]\+/#/g' | \
        sort | \
        uniq -c | \
        sort -k1,1nr -k2,2 | \
        head -n "$n" | \
        sed 's/^ *//'
}