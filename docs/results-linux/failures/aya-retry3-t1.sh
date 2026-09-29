retry3() {
  local -i attempts=0
  local -i max_attempts=3
  local -i exit_code=0

  until [[ $attempts -ge $max_attempts ]]; do
    (
      attempts=$((attempts + 1))
      "$@"
      exit_code=$?
    ) || :
    if [[ $exit_code -eq 0 ]]; then
      break
    fi
  done

  exit $exit_code
}