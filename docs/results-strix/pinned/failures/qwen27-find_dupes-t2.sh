find_dupes() {
  local dir="$1"
  local -a files=()
  local -A md5_map=()
  local f md5

  # Collect all regular files
  while IFS= read -r -d '' f; do
    files+=("$f")
  done < <(find "$dir" -type f -print0)

  # Compute MD5 for each file and group by hash
  for f in "${files[@]}"; do
    md5=$(md5 -r "$f" | awk '{print $1}')
    if [[ -n "${md5_map[$md5]+x}" ]]; then
      md5_map[$md5]="${md5_map[$md5]} $f"
    else
      md5_map[$md5]="$f"
    fi
  done

  # Collect groups with 2+ files
  local -a groups=()
  local group
  for group in "${md5_map[@]}"; do
    # Count files in group
    local -a group_files=()
    read -ra group_files <<< "$group"
    if (( ${#group_files[@]} >= 2 )); then
      # Sort the files in this group
      local sorted_group
      sorted_group=$(printf '%s\n' "${group_files[@]}" | sort)
      groups+=("$sorted_group")
    fi
  done

  # Sort groups by first path
  if (( ${#groups[@]} > 0 )); then
    printf '%s\n' "${groups[@]}" | sort | while IFS= read -r line; do
      printf '%s\n' "$line"
    done
  fi
}