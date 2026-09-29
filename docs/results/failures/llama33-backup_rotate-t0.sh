backup_rotate() {
  local dir="$1"
  local keep="$2"
  local files=("$dir"/backup-*.tar.gz)
  if (( ${#files[@]} <= keep )); then
    return
  fi
  local sorted_files=($(printf '%s\n' "${files[@]}" | sort -r))
  local to_delete=("${sorted_files[@]:keep}")
  for file in "${to_delete[@]}"; do
    echo "${file##*/}"
    rm "$file"
  done
}