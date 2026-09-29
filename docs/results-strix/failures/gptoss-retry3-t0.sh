retry3() {
  local attempts=3
  local status
  for ((i=1; i<=attempts; i++)); do
    "$@" || status=$?
    if [[ ${status:-0} -eq 0 ]]; then
      return 0
    fi
  done
  return ${status:-1}
}