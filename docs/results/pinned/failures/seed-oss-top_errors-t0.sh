top_errors() {
  awk '/ERROR: / { sig = substr($0, index($0, "ERROR: ") + 7); gsub(/[0-9]+/, "#", sig); print sig }' "$1" | sort | uniq -c | sort -k1nr -k2 | head -n "$2"
}