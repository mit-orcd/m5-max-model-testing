top_errors() {
    local logfile=$1 n=$2
    if [[ ! -f $logfile || ! $n =~ ^[0-9]+$ || $n -le 0 ]]; then
        return
    fi
    grep -E 'ERROR: ' "$logfile" | cut -d ' ' -f2- | sed 's/[0-9]\+/#/g' | sort | uniq -c | sort -k1,1nr -k2,2 | head -n "$n" | while read count sig; do echo "$count $sig"; done
}