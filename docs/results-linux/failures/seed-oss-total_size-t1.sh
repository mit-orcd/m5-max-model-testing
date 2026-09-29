total_size() {
  local dir="$1"
  if [ ! -d "$dir" ]; then
    echo 0
    return
  fi
  find "$dir" -type f -printf '%s\n' | awk '{sum += $1} END {print sum}'
}