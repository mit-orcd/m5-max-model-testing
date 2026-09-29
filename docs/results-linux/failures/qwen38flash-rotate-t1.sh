rotate() {
  local file="$1"
  local current="$file"
  local next

  while [ -e "$current" ]; do
    next="${current}.$(( ${#file} == ${#current} ? 1 : ${#current} + 1 ))"
    # Actually simpler: next is current.1
    next="${current}.1"
    mv "$current" "$next"
    current="$next"
  done
}