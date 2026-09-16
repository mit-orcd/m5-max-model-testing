top_errors() {
  local logfile="$1" n="$2"
  [ -z "$logfile" ] || [ -z "$n" ] && return
  grep 'ERROR: ' "$logfile" | \
  sed -E 's/^.*ERROR: //; s/[0-9]+/#/g' | \
  sort | \
  uniq -c | \
  sort -k1,1nr -k2 | \
  head -n "$n" | \
  awk '{print $1, $2}'
}