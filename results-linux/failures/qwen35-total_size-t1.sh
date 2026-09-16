total_size() {
  local dir="$1"
  local sum=0
  local item
  while IFS= read -r -d '' item; do
    if [[ -f "$item" ]]; then
      sum=$((sum + $(stat -f%z -- "$item" 2>/dev/null || echo 0)))
    fi
  done < <(find "$dir" -type f -print0 2>/dev/null)
  printf '%d\n' "$sum"
}