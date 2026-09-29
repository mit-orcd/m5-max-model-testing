find_dupes() {
  local dir="$1"
  local -A hash_to_paths
  while IFS= read -r -d '' file; do
    hash=$(md5 -r "$file")
    hash_to_paths[$hash]+=" $file"
  done < <(find "$dir" -type f -print0)

  for hash in "${!hash_to_paths[@]}"; do
    paths=(${hash_to_paths[$hash]})
    if (( ${#paths[@]} > 1 )); then
      printf '%s\n' "${paths[@]}" | sort | tr '\n' ' '
      echo
    fi
  done | sort
}