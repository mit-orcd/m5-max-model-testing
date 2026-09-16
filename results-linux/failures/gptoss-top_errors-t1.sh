top_errors() {
    local logfile="$1"
    local n="$2"
    [[ -z "$logfile" || -z "$n" ]] && return

    awk -F'ERROR: ' '/ERROR: /{print $2}' "$logfile" |
        sed 's/[0-9]\+/#/g' |
        sort |
        uniq -c |
        sort -k1,1nr -k2,2 |
        head -n "$n"
}