top_errors() {
    local logfile="$1"
    local n="$2"
    grep -o 'ERROR: .*' "$logfile" | sed 's/ERROR: //' | sed 's/[0-9]\+/#/g' | sort | uniq -c | sort -nr -k1,1 -k2,2 | head -n "$n" | awk '{print $1, $0}'
}