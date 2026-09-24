retry3() {
  local attempts=3
  local delay=1
  local cmd=("$@")
  local exit_code=0

  for ((i=1; i<=attempts; i++)); do
    "${cmd[@]}"
    exit_code=$?
    if [[ $exit_code -eq 0 ]]; then
      break
    fi
    if [[ $i -lt $attempts ]]; then
      sleep "$delay"
    fi
  done

  exit $exit_code
}