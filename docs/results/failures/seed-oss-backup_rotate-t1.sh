backup_rotate() {
  local dir="$1" keep="$2"
  shopt -s nullglob
  local backups=( "$dir"/backup-*.tar.gz )
  shopt -u nullglob
  local num_backups=${#backups[@]}
  (( num_backups <= keep )) && return
  local sorted=($(printf "%s\n" "${backups[@]##*/}" | sort -r))
  local to_delete=("${sorted[@]:keep}")
  for file in "${to_delete[@]}"; do
    echo "$file"
    rm -f -- "$dir/$file"
  done
}