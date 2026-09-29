backup_rotate() {
  local dir=$1
  local keep=$2
  local files=("$dir"/backup-*.tar.gz)
  local delete_count=$(( ${#files[@]} - keep ))
  if (( delete_count > 0 )); then
    printf '%s\n' "${files[@]:0:delete_count}" | sort -r | while read -r file; do
      rm "$file"
      echo "${file##*/}"
    done
  fi
}