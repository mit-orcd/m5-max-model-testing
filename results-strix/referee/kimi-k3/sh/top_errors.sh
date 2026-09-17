top_errors() {
  grep 'ERROR: ' "$1" | sed -e 's/.*ERROR: //' -e 's/[0-9][0-9]*/#/g' \
    | sort | uniq -c | sort -k1,1nr -k2 | sed 's/^ *//' | head -n "$2"
}
