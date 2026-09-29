largest_file() {
  find "$1" -type f -print0 | du -h0 | sort -z -r -k2 | head -z -n1 | cut -z -d$'\t' -f2-
}