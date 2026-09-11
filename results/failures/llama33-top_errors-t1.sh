top_errors() {
  local logfile="$1"
  local n="$2"

  grep "ERROR: " "$logfile" |
  sed 's/[0-9]/#/g' |
  awk '{print $0}' |
  sort |
  uniq -c |
  sort -rn -k1,1 -k2 |
  head -n "$n"
}