largest_file() {
  local dir="$1"
  find "$dir" -type f -printf '%s\t%p\0' | sort -z -n -k1,1 | tail -z -n1 | cut -z -f2- | tr '\0' '\n'
}