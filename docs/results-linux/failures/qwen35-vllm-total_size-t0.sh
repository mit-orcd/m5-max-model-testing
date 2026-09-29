total_size() {
  local dir="$1"
  local sum=0
  while IFS= read -r -d '' file; do
    if [[ -f "$file" ]]; then
      local size
      size=$(stat -f %z -- "$file" 2>/dev/null) || continue
      ((sum += size))
    fi
  done < <(find "$dir" -type f -print0 2>/dev/null)
  printf '%d\n' "$sum"
}