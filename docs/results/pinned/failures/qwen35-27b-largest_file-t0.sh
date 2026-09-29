largest_file() {
  find "$1" -type f -printf '%s\t%p\n' | sort -rn | head -n 1 | cut -f2
}