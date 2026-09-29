total_size() {
  local dir="$1"
  local sum
  sum=$(find "$dir" -type f -print0 2>/dev/null | \
        while IFS= read -r -d '' file; do
          stat -f %z "$file" 2>/dev/null
        done | awk '{s+=$1} END{print s}')
  printf "%s" "$sum"
}