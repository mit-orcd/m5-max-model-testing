find_dupes() {
  local dir="$1"
  local -a files=()
  local -A md5_map=()
  local -A file_by_md5=()

  # Collect all regular files
  while IFS= read -r -d '' f; do
    files+=("$f")
  done < <(find "$dir" -type f -print0 2>/dev/null)

  # Compute MD5 for each file and group them
  local f md5
  for f in "${files[@]}"; do
    md5=$(md5 -r "$f" 2>/dev/null | awk '{print $1}')
    if [[ -z "$md5" ]]; then
      continue
    fi
    if [[ -n "${file_by_md5[$md5]:-}" ]]; then
      file_by_md5[$md5]="${file_by_md5[$md5]} $f"
    else
      file_by_md5[$md5]="$f"
    fi
  done

  # Collect groups with 2+ files
  local -a groups=()
  local md5 files_in_group first_path
  for md5 in "${!file_by_md5[@]}"; do
    files_in_group="${file_by_md5[$md5]}"
    # Count files
    local -a arr=($files_in_group)
    if (( ${#arr[@]} >= 2 )); then
      # Sort the paths
      local sorted
      sorted=$(printf '%s\n' "${arr[@]}" | LC_ALL=C sort)
      first_path=$(printf '%s\n' "$sorted" | head -n1)
      groups+=("$first_path|$sorted")
    fi
  done

  # Sort groups by first path
  if (( ${#groups[@]} > 0 )); then
    printf '%s\n' "${groups[@]}" | LC_ALL=C sort -t'|' -k1,1 | while IFS='|' read -r _ sorted; do
      # Convert newline-separated to space-separated
      local line
      line=$(printf '%s' "$sorted" | tr '\n' ' ' | sed 's/ $//')
      printf '%s\n' "$line"
    done
  fi
}