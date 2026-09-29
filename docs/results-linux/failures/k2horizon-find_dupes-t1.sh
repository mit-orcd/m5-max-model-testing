find_dupes() {
  local dir="$1"
  [ -d "$dir" ] || { echo "find_dupes: $dir: not a directory" >&2; return 1; }
  local -A hashes=()
  local file hash
  while IFS= read -r file; do
    hash=$(md5 -q "$file" 2>/dev/null) || continue
    hashes["$hash"]+="$file"$'\n'
  done < <(find "$dir" -type f -print 2>/dev/null)
  local group
  for hash in "${!hashes[@]}"; do
    group=$(printf '%s' "${hashes[$hash]}" | sort)
    local count
    count=$(printf '%s\n' "$group" | grep -c .)
    if [ "$count" -ge 2 ]; then
      printf '%s\n' "$group"
    fi
  done | sort
}