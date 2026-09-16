total_size() {
  local dir="$1"
  local sum=0
  find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
    local size=$(stat -f '%z' "$file" 2>/dev/null)
    ((sum += size))
  done
  echo "$sum"
}