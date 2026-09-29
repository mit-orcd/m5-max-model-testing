total_size() {
  find "$1" -type f -exec stat -f %z {} + 2>/dev/null | awk '{sum += $1} END {print sum + 0}'
}