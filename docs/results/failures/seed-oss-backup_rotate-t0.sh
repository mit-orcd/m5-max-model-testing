backup_rotate() {
  local dir="$1" keep="$2"
  shopt -s nullglob
  local files=("$dir"/backup-*.tar.gz)
  shopt -u nullglob
  local num_files=${#files[@]}
  (( num_files <= keep )) && return
  local sorted=($(printf "%s\n" "${files[@]##*/}" | sort -r))
  local delete=("${sorted[@]:keep}")
  for file in "${delete[@]}"; do
    rm -f -- "$dir/$file"
    printf "%s\n" "$file"
  done
}