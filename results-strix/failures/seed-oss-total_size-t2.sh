total_size() {
  find "$1" -type f -printf "%s\n" 2>/dev/null | awk '{sum += $1} END {print sum}'
}