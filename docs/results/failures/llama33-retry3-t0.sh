retry3() {
  local max_attempts=3
  local attempt=0
  while [ $attempt -lt $max_attempts ]; do
    if "$@"; then
      return 0
    fi
    attempt=$((attempt + 1))
  done
  return $?
}