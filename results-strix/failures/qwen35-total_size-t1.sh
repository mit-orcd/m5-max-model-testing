total_size() {
  local dir="$1"
  local total=0
  if [ -d "$dir" ]; then
    while IFS= read -r -d '' file; do
      if [ -f "$file" ]; then
        size=$(stat -f%z "$file" 2>/dev/null || echo 0)
        total=$((total + size))
      fi
    done < <(find "$dir" -type f -print0)
  fi
  echo "$total"
}