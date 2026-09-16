top_errors() {
  local logfile="$1" n="$2"
  [[ ! -f "$logfile" ]] && return
  grep -o 'ERROR: .*' "$logfile" | sed 's/ERROR: //; s/\([0-9]\{1,\}\)/#/g' | sort | uniq -c | sort -k1nr -k2 | head -n "$n" | awk '{print $1 " " $2}'
}