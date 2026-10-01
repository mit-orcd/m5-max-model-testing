find_dupes() {
  local dir="$1"
  local tmpfile=$(mktemp)
  local tmpout=$(mktemp)

  # Find all regular files, compute MD5, store "md5 path"
  find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | while IFS= read -r line; do
    # md5 -r outputs: <hash> <filename>
    # The filename may contain spaces, so we need to be careful
    # Format: first field is hash, rest is filename
    local hash="${line%% *}"
    local filepath="${line#* }"
    printf '%s\t%s\n' "$hash" "$filepath"
  done > "$tmpfile"

  # Sort by hash, then by path
  sort -t$'\t' -k1,1 -k2,2 "$tmpfile" > "$tmpout"

  # Group by hash and print groups with 2+ files
  local current_hash=""
  local current_paths=()

  while IFS=$'\t' read -r hash filepath; do
    if [[ "$hash" != "$current_hash" ]]; then
      # Print previous group if it had 2+ files
      if [[ ${#current_paths[@]} -ge 2 ]]; then
        # Sort paths within the group
        local sorted_paths
        sorted_paths=$(printf '%s\n' "${current_paths[@]}" | sort)
        # Join with spaces
        local joined
        joined=$(echo "$sorted_paths" | tr '\n' ' ')
        # Trim trailing space
        joined="${joined% }"
        echo "$joined"
      fi
      current_hash="$hash"
      current_paths=("$filepath")
    else
      current_paths+=("$filepath")
    fi
  done < "$tmpout"

  # Print last group
  if [[ ${#current_paths[@]} -ge 2 ]]; then
    local sorted_paths
    sorted_paths=$(printf '%s\n' "${current_paths[@]}" | sort)
    local joined
    joined=$(echo "$sorted_paths" | tr '\n' ' ')
    joined="${joined% }"
    echo "$joined"
  fi

  rm -f "$tmpfile" "$tmpout"
}