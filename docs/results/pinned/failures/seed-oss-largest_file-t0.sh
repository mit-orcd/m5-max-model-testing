largest_file() {
  find "$1" -type f -printf "%s\t%p\n" | sort -k1,1nr | head -n1 | cut -f2-
}