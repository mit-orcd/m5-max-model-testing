largest_file() {
  find "$1" -type f -exec stat --format='%s %n' {} + 2>/dev/null | sort -nr | head -n 1 | cut -d' ' -f2-
}