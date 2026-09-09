top_errors() {
    local logfile="$1"
    local n="$23"
    
    if [[ -f "$logfile" ]]; then
        grep -oP 'ERROR: \K[^[:space:]]+' "$logfile" | while read -r signature; do
            # Replace runs of digits with '#'
            sanitized=$(echo "$signature" | sed 's/[[:digit:]]+/#/g')
            echo "$sanitized"
        done | sort | uniq -c | sort -rn -k 1 | head -n "$n"
    fi
}