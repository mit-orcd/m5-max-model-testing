backup_rotate() {
  local dir="$1"
  local keep="$2"
  local files=()
  local count=0
  local deleted=()

  while IFS= read -r -d $'\0' file; do
    files+=("$file")
  done < <(find "$dir" -maxdepth 1 -name "backup-*.tar.gz" -print0 | sort -z -r)

  count=${#files[@]}

  if (( keep >= count )); then
    exit 0
  fi

  for (( i=keep; i<count; i++ )); do
    deleted+=("${files[i]##*/}")
  done

  for (( i=count-1; i>=keep; i-- )); do
    rm -f "${files[i]}"
  done

  for (( i=0; i<${#deleted[@]}; i++ )); do
    echo "${deleted[i]}"
  done
}