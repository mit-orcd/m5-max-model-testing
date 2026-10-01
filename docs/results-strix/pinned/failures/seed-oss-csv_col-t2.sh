csv_col() {
  read -r -a header
  local col_idx=-1
  for i in "${!header[@]}"; do
    [[ "${header[$i]}" == "$1" ]] && { col_idx=$i; break; }
  done
  while IFS=, read -r -a fields; do
    echo "${fields[$col_idx]}"
  done
}