top_errors() {
    local logfile="$1"
    local n="$2"
    grep -o 'ERROR: .*' "$logfile" 2>/dev/null | \
        sed 's/ERROR: //; s/[0-9]\+#/g' | \
        sort | uniq -c | sort -k1,1nr -k2 | head -n "$n" | \
        awk '{count=$1; $1=""; sub(/^ /, ""); print count " " $0}'
}