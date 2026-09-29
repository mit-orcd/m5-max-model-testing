retry3() {
  local attempts=0
  local max_attempts=3
  local exit_code=1
  while (( attempts < max_attempts )); do
    "$@" && exit_code=0 && break
    (( exit_code = $? ))
    (( attempts++ ))
    if (( attempts < max_attempts )); then
      sleep 1
    fi
  done
  exit $exit_code
}