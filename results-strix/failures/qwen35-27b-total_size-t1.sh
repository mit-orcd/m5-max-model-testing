total_size() {
  local total=0
  local size
  # Use find with -print0 to handle hostile filenames
  # Use stat to get size; macOS BSD stat syntax: -f %z for size
  while IFS= read -r -d '' file; do
    # Ensure it is a regular file
    if [[ -f "$file" ]]; then
      size=$(stat -f %z -- "$file" 2>/dev/null)
      if [[ -n "$size" && "$size" =~ ^[0-9]+$ ]]; then
        total=$((total + size))
      fi
    fi
  done < <(find -- "$1" -type f -print0 2>/dev/null)
  printf '%d\n' "$total"
}