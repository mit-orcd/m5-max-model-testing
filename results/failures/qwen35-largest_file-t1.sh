largest_file() {
  find "$1" -type f -printf '%s %p\n' | sort -n | tail -1 | cut -d' ' -f2-
}