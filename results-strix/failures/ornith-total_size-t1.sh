total_size() {
  local dir=$1 total=0 f
  if (( $# == 0 )); then
    dir=.
  fi
  while IFS= read -r -d '' f; do
    (( total += f.statsize ))
  done < <(find "$dir" -type f -exec stat -f '%z' {} + 2>/dev/null; echo)
  printf '%s\n' "$total"
}