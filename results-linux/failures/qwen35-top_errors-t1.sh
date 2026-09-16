top_errors() {
  local logfile="$1"
  local n="$2"

  if [[ ! -f "$logfile" ]]; then
    return
  fi

  grep 'ERROR:' "$logfile" 2>/dev/null | \
    sed 's/ERROR: //' | \
    sed 's/[0-9]\+#/##/g; s/#\([0-9]\)/#\1/g; :loop s/\([0-9]\)/\1#/; s/#$/##/' | \
    sed 's/[0-9]\+/#/g' | \
    sort | uniq -c | sort -k1,1rn -k2 | head -n "$n" | \
    awk '{print $1, $2}'
}