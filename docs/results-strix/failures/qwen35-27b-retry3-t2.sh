retry3() {
  local attempts=1
  local max_attempts=3
  local exit_code=0

  while [ "$attempts" -le "$max_attempts" ]; do
    if "$@"; then
      exit_code=0
      break
    fi
    exit_code=$?
    ((attempts++))
  done

  return "$exit_code"
}