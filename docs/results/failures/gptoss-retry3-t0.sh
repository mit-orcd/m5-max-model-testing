retry3() {
  local attempts=3
  local status
  for ((i=1; i<=attempts; i++)); do
    "$@"
    status=$?
    if [ $status -eq 0 ]; then
      exit 0
    fi
  done
  exit "$status"
}