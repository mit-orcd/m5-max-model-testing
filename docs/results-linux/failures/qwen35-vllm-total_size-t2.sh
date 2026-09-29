total_size() {
  local dir="$1"
  local sum=0
  
  if [[ -d "$dir" ]]; then
    while IFS= read -r -d '' file; do
      if [[ -f "$file" && ! -L "$file" ]]; then
        local size
        size=$(stat -f %z "$file" 2>/dev/null) || continue
        ((sum += size))
      fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
  fi
  
  printf '%s\n' "$sum"
}