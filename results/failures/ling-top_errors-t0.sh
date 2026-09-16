top_errors() {
    awk -F'ERROR: ' '/ERROR: / {gsub(/[0-9]+/, "#", $2); print $2}' "$1" |
    sort | uniq -c | sort -k1,1nr -k2,2 | head -n "$2" |
    awk '{print $1, $2}'
}