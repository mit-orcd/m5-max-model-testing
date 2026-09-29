csv_col() {
  IFS= read -r header || return 1
  IFS=',' read -ra header_cols <<< "$header"
  local col_idx=-1
  for i in "${!header_cols[@]}"; do
    [[ "${header_cols[$i]}" == "$1" ]] && { col_idx=$i; break; }
  done
  [[ $col_idx -lt 0 ]] && return 1
  tail -n +2 | while IFS= read -r line; do
    IFS=',' read -ra fields <<< "$line"
    echo "${fields[$col_idx]}"
  done
}