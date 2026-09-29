top_errors() {
  awk '/ERROR: / { pos = index($0, "ERROR: "); print substr($0, pos + 7) }' "$1" |
    sed 's/[0-9]\+/#/g' |
    sort |
    uniq -c |
    sort -k1,1nr -k2,2 |
    head -n "$2"
}