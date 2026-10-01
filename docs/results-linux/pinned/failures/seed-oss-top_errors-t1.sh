top_errors() {
  awk -F 'ERROR: ' '{if (NF > 1) print $2}' "$1" |
    sed -E 's/[0-9]+/#/g' |
    sort |
    uniq -c |
    sort -k1,1nr -k2,2 |
    head -n "$2"
}