retry3() {
  local attempts=3
  local exit_code=1
  for ((i=1;i<=attempts;i++)); do
    "$@" || exit_code=$?
    if [[ ${exit_code:-0} -eq 0 ]]; then
      exit 0
    fi
  done
  exit ${exit_code:-1}
}