top_errors() {
  local logfile="$1" n="$2"
  [[ ! -f "$logfile" ]] && return
  grep -o 'ERROR: .*' "$logfile" | sed -E 's/ERROR: (.*)/\1/' | sed -E 's/[0-9]+/#/g' | sort | uniq -c | sort -k1nr -k2 | head -n "$n" | awk '{print $1, $0}' | cut -d' ' -f1,2-
}