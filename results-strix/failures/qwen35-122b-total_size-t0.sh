total_size() {
  local dir="$1"
  if [[ ! -d "$dir" ]]; then
    echo 0
    return
  fi
  local total=0
  local size
  while IFS= read -r -d '' file; do
    if [[ -f "$file" && ! -L "$file" ]]; then
      size=$(stat -f '%z' -- "$file" 2>/dev/null) || size=$(stat -f '%z' "$file" 2>/dev/null)
      if [[ -n "$size" && "$size" =~ ^[0-9]+$ ]]; then
        total=$((total + size))
      fi
    fi
  done < <(find "$dir" -print0 2>/dev/null)
  echo "$total"
}