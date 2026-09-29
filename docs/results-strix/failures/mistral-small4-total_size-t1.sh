total_size() {
  local dir="$1"
  local size=0
  while IFS= read -r -d '' file; do
    if [[ -f "$file" ]]; then
      size=$((size + $(stat -f "%z" "$file"))
    fi
  done < <(find "$dir" -type f -print0)
  echo "$size"
}