retry3() {
  local cmd=("$@")
  local attempt
  for attempt in 1 2 3; do
    "${cmd[@]}"
    if [[ $? -eq 0 ]]; then
      return 0
    fi
  done
  return $?
}