csv_col() {
  local name=$1
  local header=($(head -n 1))
  local index=-1
  for i in "${!header[@]}"; do
    if [ "${header[$i]}" = "$name" ]; then
      index=$i
      break
    fi
  done
  if [ $index -eq -1 ]; then
    return
  fi
  tail -n +2 | cut -d, -f$((index + 1))
}