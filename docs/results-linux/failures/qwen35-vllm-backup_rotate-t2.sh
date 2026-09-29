backup_rotate() {
  local dir="$1"
  local keep="$2"
  
  # Check arguments
  if [[ -z "$dir" || -z "$keep" ]]; then
    return 1
  fi

  # Ensure keep is a number
  if ! [[ "$keep" =~ ^[0-9]+$ ]]; then
    return 1
  fi

  # Find all matching files, sort them lexicographically (oldest first)
  local files=()
  while IFS= read -r -d '' file; do
    files+=("$file")
  done < <(find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' -printf '%f\n' | sort)

  local count=${#files[@]}
  
  # If count <= keep, do nothing
  if (( count <= keep )); then
    return 0
  fi

  # Calculate how many to delete
  local to_delete=$(( count - keep ))

  # Delete the oldest ones and print their names
  for (( i=0; i<to_delete; i++ )); do
    local fname="${files[$i]}"
    rm -- "$dir/$fname"
    echo "$fname"
  done
}