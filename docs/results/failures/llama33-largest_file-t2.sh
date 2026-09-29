largest_file() {
  find "$1" -type f -print0 | du -h --files0-from - | sort -rh | head -1 | cut -d$'\t' -f2-
}