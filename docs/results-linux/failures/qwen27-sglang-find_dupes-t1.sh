find_dupes() {
  local dir="$1"
  local -a files=()
  local -A md5_map=()

  # Collect all regular files
  while IFS= read -r -d '' f; do
    files+=("$f")
  done < <(find "$dir" -type f -print0 | sort -z)

  # Compute MD5 for each file
  for f in "${files[@]}"; do
    local hash
    hash=$(md5 -r "$f" | cut -d ' ' -f 1)
    md5_map["$hash"]+="$f"$'\n'
  done

  # Collect groups with 2+ files
  local -a groups=()
  local hash group_lines
  for hash in "${!md5_map[@]}"; do
    group_lines="${md5_map[$hash]}"
    local -a group_files=()
    while IFS= read -r line; do
      [[ -n "$line" ]] && group_files+=("$line")
    done <<< "$group_lines"
    if (( ${#group_files[@]} >= 2 )); then
      # Sort the group files
      local sorted_group
      sorted_group=$(printf '%s\n' "${group_files[@]}" | sort)
      # Build space-separated line
      local line=""
      while IFS= read -r pf; do
        if [[ -z "$line" ]]; then
          line="$pf"
        else
          line="$line $pf"
        fi
      done <<< "$sorted_group"
      groups+=("$line")
    fi
  done

  # Sort groups by first path
  if (( ${#groups[@]} > 0 )); then
    printf '%s\n' "${groups[@]}" | sort
  fi
}