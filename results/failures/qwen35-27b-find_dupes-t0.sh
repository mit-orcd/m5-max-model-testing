find_dupes() {
  local dir="$1"
  [[ -z "$dir" ]] && return 1

  # Create a temporary file to store hash:path pairs
  local tmpfile
  tmpfile=$(mktemp)
  trap "rm -f '$tmpfile'" EXIT

  # Find regular files and compute MD5 hash
  # macOS 'find' does not support -printf, so we use -exec
  # macOS 'md5 -r' outputs: "MD5 (filename) = hash"
  # We parse this to get "hash filename"
  find "$dir" -type f -exec md5 -r {} + 2>/dev/null | \
    sed 's/^MD5 (.* = //' | \
    sort > "$tmpfile"

  # Process the sorted list to find duplicates
  local prev_hash=""
  local prev_path=""
  local group=()

  while IFS= read -r line; do
    local hash="${line%% *}"
    local path="${line#* }"

    if [[ "$hash" == "$prev_hash" ]]; then
      group+=("$path")
    else
      # Output previous group if it has 2+ files
      if [[ ${#group[@]} -ge 2 ]]; then
        # Sort paths within the group and print
        printf '%s\n' "${group[@]}" | sort | tr '\n' ' ' | sed 's/ $/\n/'
      fi
      # Start new group
      group=("$path")
      prev_hash="$hash"
    fi
    prev_path="$path"
  done < "$tmpfile"

  # Handle the last group
  if [[ ${#group[@]} -ge 2 ]]; then
    printf '%s\n' "${group[@]}" | sort | tr '\n' ' ' | sed 's/ $/\n/'
  fi
}