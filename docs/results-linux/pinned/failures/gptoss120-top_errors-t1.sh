top_errors() {
    local logfile=$1 n=$2
    [[ -r $logfile && $n =~ ^[0-9]+$ ]] || return

    grep -a 'ERROR: ' "$logfile" |
    sed -e 's/.*ERROR: //' -e 's/[0-9]\+/#/g' |
    sort |
    uniq -c |
    sort -k1,1nr -k2,2 |
    head -n "$n"
}