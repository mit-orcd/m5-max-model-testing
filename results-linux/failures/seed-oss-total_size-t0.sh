total_size() {
  if [ ! -d "$1" ]; then
    echo 0
    return
  fi
  find -- "$1" -type f -printf '%s\n' 2>/dev/null | awk '{sum += $1} END {print sum}'
}