top_errors() {
  grep -F 'ERROR: ' "$1" | awk '{ print substr($0, index($0, "ERROR: ") + 7) }' | sed 's/[0-9]\+/#/g' | sort | uniq -c | sort -k1,1nr -k2 | head -n "$2"
}