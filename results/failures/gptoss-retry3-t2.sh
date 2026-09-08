retry3() {
  local last_exit=0
  for i in 1 2 3; do
    "$@" || last_exit=$?
    if (( last_exit == 0 )); then
      return 0
    fi
  done
  return "$last_exit"
}