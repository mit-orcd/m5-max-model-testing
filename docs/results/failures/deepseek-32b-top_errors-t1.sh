function top_errors {
    local logfile="$1"
    local n="$2"
    grep 'ERROR:' "$logfile" | sed -e 's/.*ERROR://; s/[0-9]\+/#/g' | sort | uniq -c | sort -k1,1nr -k2,2 | head -n "$n"
}