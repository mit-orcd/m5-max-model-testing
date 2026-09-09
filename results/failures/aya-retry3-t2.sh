retry3() {
  local attempts=0
  local -r cmd="$@"
  local exit_code

  until [ $attempts -ge 3 ]; do
    exit_code=$?
    attempts=$((attempts + 1))
    $@
  done
  exit $exit_code
}