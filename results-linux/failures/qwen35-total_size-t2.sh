total_size() {
  find "$1" -type f -print0 | xargs -0 -I {} printf '%s\n' "{}" | while IFS= read -r -d '' file; do
    stat -f%z "$file" 2>/dev/null || true
  done | awk '{s+=$1} END {print s+0}'
}