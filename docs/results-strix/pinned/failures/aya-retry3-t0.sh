retry3() {
  local -i attempts=0
  local -i max_attempts=3
  local -i exit_code=0

  until [[ $attempts -ge $max_attempts ]]; do
    exit_code=$?
    attempts=$((attempts + 1))
    if [[ $exit_code -eq 0 ]]; then
      return 0
    fi
    sleep 1
  done

  return $exit_code
}