retry3() {
  local attempts=3
  local i=1
  local exit_code=0
  while (( i <= attempts )); do
    "$@" || exit_code=$?
    if (( exit_code == 0 )); then
      exit 0
    fi
    (( i++ ))
  done
  exit "$exit_code"
}