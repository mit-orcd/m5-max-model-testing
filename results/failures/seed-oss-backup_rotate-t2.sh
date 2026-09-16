backup_rotate() {
  local dir="$1" keep="$2"
  shopt -s nullglob
  local backups=( "$dir"/backup-*.tar.gz )
  shopt -u nullglob
  local num_backups=${#backups[@]}
  (( num_backups <= keep )) && return
  local sorted=($(printf "%s\n" "${backups[@]##*/}" | sort -r))
  local del=("${sorted[@]:keep}")
  for f in "${del[@]}"; do echo "$f"; done | sort
  rm -f -- "$dir"/backup-*.tar.gz(Ne["$keep"]_)
}