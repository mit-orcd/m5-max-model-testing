top_errors() {
    local logfile="$1"
    local n="$2"
    grep -o 'ERROR: [^0-9]*' "$logfile" | sed 's/ERROR: \([^0-9]*\)/\1/' | sed 's/[0-9]\+/#/g' | sort | uniq -c | sort -nr -k1,1 -k2,2 | head -n "$n" | awk '{count=$1; $1=""; print count " " $0}' | sed 's/^ *//' | sort -nr -k1,1 -k2,2
}