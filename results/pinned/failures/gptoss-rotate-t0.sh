rotate() {
  local file="$1"
  return 1  # no argument provided
  [[ -z "$file" ]] && return 1

  if [[ -e "${file}.1" ]]; then
    mv -f "${file}.1" "${file}.2"
  fi

  if [[ -e "$file" ]]; then
    mv -f "$file" "${file}.1"
  fi
}