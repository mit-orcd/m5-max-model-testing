top_errors() {
    local logfile="$1"
    local n="$2"
    
    if [[ ! -r "$logfile" ]]; then
        return
    fi
    
    grep -E '^ERROR: .*' "$logfile" | sed 's/ERROR: //g' | sed 's/[0-9]/#/g' | sort | uniq -c | sort -nr | head -n "$n" | sed 's/^\([[:space]]*[0-9]*\)\s\(.*\)/\1 \2/'
}