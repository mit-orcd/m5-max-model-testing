largest_file() {
  find "$1" -type f -printf '%s\t%p\n' | sort -t$'\t' -k1,1 -rn | head -n1 | cut -f2-
}