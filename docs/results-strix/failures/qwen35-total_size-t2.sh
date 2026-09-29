total_size() {
  local dir="$1"
  local total=0
  while IFS= read -r -d '' file; do
    if [[ -f "$file" && ! -L "$file" ]]; then
      total=$((total + $(stat -f "%z" -- "$file" 2>/dev/null || echo 0)))
    fi
  done < <(find "$dir" -type f -print0 2>/dev/null)
  echo "$total"
}