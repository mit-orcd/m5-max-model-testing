retry3() {
  local attempts=0
  local max_attempts=3
  local cmd=("$@")
  local exit_code=0

  while [ $attempts -lt $max_attempts ]; do
    "${cmd[@]}"
    exit_code=$?
    if [ $exit_code -eq 0 ]; then
      break
    fi
    ((attempts++))
    if [ $attempts -lt $max_attempts ]; then
      sleep 1
    fi
  done

  exit $exit_code
}