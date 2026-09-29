total_size() {
  find "$1" -type f -exec stat -f %z {} + | awk '{sum += $1} END {print sum + 0}'
}