total_size() {
  local dir="$1"
  local total=0

  while IFS= read -r -d '' -a files; do
    for f in "${files[@]}"; do
      if [ -f "$f" ]; then
        total=$((total + $(stat -f%z "$f" 2>/dev/null || echo 0)))
      fi
    done
  done < <(find "$dir" -type f -print -exec sh -c 'echo "$@"' _ {} + 2>/dev/null)

  echo "$total"
}