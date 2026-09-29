retry3() {
  local cmd="$*" attempts=0 max_attempts=3 last_exit=0
  until $cmd || [ $attempts -eq $((max_attempts - 1)) ]; do
    ((attempts++))
    last_exit=$?
  done
  if [ $attempts -lt $max_attempts ]; then
    return 0
  else
    return $last_exit
  fi
}