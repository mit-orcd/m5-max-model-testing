retry3() {
  local attempts=0
  local -r cmd="$@"
  until [[ $attempts -ge 3 ]]; do
    attempts=$((attempts + 1))
    if eval "$cmd"; then
      return 0
    fi
  done
  return $?
}