top_errors() {
    local logfile="$1"
    local n="$2"

    grep 'ERROR: ' "$logfile" | sed 's/.*ERROR: //' | sed 's/[0-9]\+/#/g' | sort | uniq -c | sort -k1,1nr -k2 | awk -v n="$n" 'NR <= n {printf "%s %s\n", $1, substr($0, index($0, $2))}' | head -n "$n"
}