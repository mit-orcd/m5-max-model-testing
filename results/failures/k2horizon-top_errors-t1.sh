top_errors() {
    local logfile="$1" n="$2"
    grep -o 'ERROR: .*' "$logfile" \
    | sed 's/^ERROR: //' \
    | sed -E 's/[0-9]+/#/g' \
    | sort | uniq -c \
    | sort -k1,1nr -k2,2 \
    | head -n "$n"
}