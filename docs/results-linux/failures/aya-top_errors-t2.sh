function top_errors() {
    local logfile="$1"
    local n="$2"

    local errors=$(grep -c 'ERROR:' "$logfile" | awk -F':' '{print $2}')
    local signatures=$(echo "$errors" | sed 's/ERROR://; s/[0-9]\+/#/') | sort | uniq -c | sort -nr | head -n "$n")

    echo "$signatures"
}