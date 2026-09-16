largest_file() {
  find "$1" -type f -printf '%s\t%p\n' 2>/dev/null |
    sort -k1,1nr |
    sed -n '2s/^[^	]*\t//p'
}