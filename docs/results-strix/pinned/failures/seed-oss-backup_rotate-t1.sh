backup_rotate() {
  local dir="$1" keep="$2"
  local files=()
  while IFS= read -d '' -r file; do
    files+=("$file")
  done < <(find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' -print0 | sort -rz)
  local count=${#files[@]}
  (( count <= keep )) && return
  local delete_idx=$keep deleted_full=("${files[@]:$delete_idx}")
  local deleted_names=()
  for file in "${deleted_full[@]}"; do
    deleted_names+=("$(basename "$file")")
  done
  local reversed=()
  for ((i=${#deleted_names[@]}-1; i>=0; i--)); do
    reversed+=("${deleted_names[i]}")
  done
  printf '%s\n' "${reversed[@]}"
  for file in "${deleted_full[@]}"; do
    rm "$file"
  done
}