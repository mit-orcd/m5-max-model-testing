find_dupes() {
  local dir="$1"
  local -a files=()
  local f
  while IFS= read -r -d '' f; do
    files+=("$f")
  done < <(find "$dir" -type f -print0 | LC_ALL=C sort -z)

  local -A md5_map=()
  local -a md5s=()
  local path md5
  for path in "${files[@]}"; do
    md5=$(md5 -r -- "$path" 2>/dev/null)
    if [[ -n "${md5_map[$md5]+x}" ]]; then
      md5_map[$md5]+=" $path"
    else
      md5_map[$md5]="$path"
      md5s+=("$md5")
    fi
  done

  local -a groups=()
  local md5 entry
  for md5 in "${md5s[@]}"; do
    entry="${md5_map[$md5]}"
    local -a parts=()
    read -ra parts <<< "$entry"
    if (( ${#parts[@]} >= 2 )); then
      groups+=("${parts[*]}")
    fi
  done

  if (( ${#groups[@]} == 0 )); then
    return 0
  fi

  printf '%s\n' "${groups[@]}" | LC_ALL=C sort
}