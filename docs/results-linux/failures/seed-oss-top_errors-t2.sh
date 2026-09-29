top_errors() {
  local logfile="$1"
  local n="$2"
  grep -F 'ERROR: ' "$logfile" | \
    awk '{print substr($0, index($0, "ERROR: ") + 7)}' | \
    sed 's/[0-9]\+/#/g' | \
    sort | \
    uniq -c | \
    sort -k1,1nr -k2,2 | \
    head -n "$n"
}