find_dupes() {
  dir="$1"
  declare -A md5_paths
  while IFS= read -r -d '' file; do
    md5=$(md5 -r "$file")
    md5_paths[$md5]+=" $file"
  done < <(find "$dir" -type f -print0)

  for md5 in "${!md5_paths[@]}"; do
    files=(${md5_paths[$md5]})
    if (( ${#files[@]} > 1 )); then
      printf "%s\n" "${files[@]}" | sort | tr '\n' ' '
      echo
    fi
  done | sort
}