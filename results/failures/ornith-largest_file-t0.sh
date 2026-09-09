largest_file() {
  find "$1" -type f -printf '%s\t%p\n' 2>/dev/null |
    sort -n |
    tail -n1 |
    cut -f2-
}