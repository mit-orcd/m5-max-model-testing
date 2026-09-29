top_errors() {
  local logfile="$1" n="$2"
  [[ ! -f "$logfile" ]] && return
  grep -o 'ERROR: .*' "$logfile" | sed 's/ERROR: //; s/\b[0-9]\+\b/#/g' | sort | uniq -c | sort -k1nr -k2 | head -n "$n" | awk '{print $1, $0}' | cut -d' ' -f1 --complement
}