top_errors() {
    local logfile=$1
    local n=$2

    awk '/ERROR: /{
            sub(/.*ERROR: /, "")
            gsub(/[0-9]+/, "#")
            print
        }' "$logfile" |
    sort |
    uniq -c |
    sort -k1,1nr -k2,2 |
    head -n "$n"
}