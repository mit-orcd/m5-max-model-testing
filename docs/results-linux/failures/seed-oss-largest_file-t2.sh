largest_file() {
  find "$1" -type f -printf "%s\0%p\n" | sort -z -nr -k1,1 | head -n 1 | cut -z -f2-
}