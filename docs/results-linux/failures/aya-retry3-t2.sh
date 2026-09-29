retry3() {
  local -i attempts=0
  local -i status=0

  until [[ $attempts -ge 3 ]]; do
    (( attempts++ ))
    status="$?"
    if [[ $status -eq 0 ]]; then
      return 0
    fi
    sleep 1
  done
  return $status
}