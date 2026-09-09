top_errors() {
    local logfile="$1" n="$2"
    grep -o 'ERROR: .*' "$logfile" | sed 's/ERROR: //' | \
    sed 's/[0-9]\+/#/g' | sort | uniq -c | sort -nrk1,1 -k2,2 | \
    awk -v n="$n" 'NR<=n {print $1, $0}' | cut -d' ' -f2-
}