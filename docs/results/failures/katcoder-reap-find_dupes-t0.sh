find_dupes() {
  local dir="$1"
  declare -A seen
  while IFS= read -r -d '' file; do
    hash=$(md5 -r "$file" 2>/dev/null)
    if [[ -n "$hash" ]]; then
      seen["$hash"]+="$file"$'\n'
    fi
  done < <(find "$dir" -type f -print0)
  local line=""
  for hash in "${!seen[@]}"; do
    local entries
    entries=$(printf '%s' "${seen[$hash]}" | sort)
    local count
    count=$(echo "$entries" | wc -l | tr -d ' ')
    if (( count >= 2 )); then
      line+=$(echo "$entries" | tr '\n' ' ' | sed 's/ *$//')$'\n'
    fi
  done
  printf '%s' "$line" | sort
}