top_errors() {
  local logfile="$1"
  local n="$2"

  if [[ ! -f "$logfile" ]]; then
    return
  fi

  grep -o 'ERROR: [^]*' "$logfile" 2>/dev/null | \
  sed -E 's/ERROR: //' | \
  sed -E 's/[0-9]+/#/g' | \
  sort | uniq -c | \
  sort -k1,1nr -k2,2 | \
  head -n "$n" | \
  awk '{print $1, $2}' | \
  sed 's/  */ /g'
}