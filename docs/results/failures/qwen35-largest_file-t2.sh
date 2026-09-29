largest_file() {
  find "$1" -type f -printf '%s %p\n' 2>/dev/null | sort -n -k1,1 | tail -n 1 | cut -d' ' -f2-
}