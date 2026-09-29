top_errors() {
  local logfile="$1"
  local n="$2"
  awk -F'ERROR: ' '/ERROR:/ {print $2}' "$logfile" | sed 's/[0-9][0-9]*/#/g' | sort | uniq -c | sort -nr | head -n "$n" | awk '{count=$1; $1=""; print count " " substr($0, 2)}' | sort -k1,1nr -k2,2 | awk '{print $2}' | sort -t' ' -k1,1nr -k2,2 | head -n "$n"
}