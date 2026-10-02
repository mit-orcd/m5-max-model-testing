top_errors() {
  local logfile="$1"
  local n="$2"
  grep -o 'ERROR: [^0-9]*' "$logfile" | sed 's/ERROR: //' | sed 's/[0-9]\+/#/g' | sort | uniq -c | sort -nr | head -n "$n" | awk '{count=$1; $1=""; print count, substr($0, 2)}' | sort -k1,1nr -k2,2 | head -n "$n" | awk '{print $1, $2}'
}