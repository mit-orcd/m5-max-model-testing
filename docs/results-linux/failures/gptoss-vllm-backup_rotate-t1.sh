backup_rotate() {
  local dir="$1"
  local keep="$2"
  local files=()
  while IFS= read -r -d '' f; do
    files+=("$f")
  done < <(find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' -print0)

  local count=${#files[@]}
  (( count <= keep )) && return

  mapfile -t sorted < <(printf '%s\n' "${files[@]}" | sort)

  local delete_count=$((count - keep))
  for ((i=0; i<delete_count; i++)); do
    local f="${sorted[i]}"
    rm -f -- "$f"
    echo "$(basename "$f")"
  end
}