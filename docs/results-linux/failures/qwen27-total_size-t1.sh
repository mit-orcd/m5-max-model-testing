total_size() {
  local dir="$1"
  local total=0
  local -a files=()
  while IFS= read -r -d '' f; do
    files+=("$f")
  done < <(find "$dir" -type f -print0)
  for f in "${files[@]}"; do
    local size
    size=$(stat -f%z "$f")
    total=$((total + size))
  done
  printf '%d\n' "$total"
}