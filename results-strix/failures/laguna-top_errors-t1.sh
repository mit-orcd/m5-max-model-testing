top_errors() {
    local logfile="$1"
    local n="$2"
    awk '/ERROR: / {
        sig = substr($0, index($0, "ERROR: ") + 7)
        gsub(/[0-9]+/, "#", sig)
        print sig
    }' "$logfile" | sort | uniq -c | sort -k1,1rn -k2,2 | head -n "$n" | awk '{print $1, $2}'
}