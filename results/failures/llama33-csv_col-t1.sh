csv_col() {
  local name=$1
  local header=()
  read -r -a header
  local index=-1
  for ((i=0; i<${#header[@]}; i++)); do
    if [ "${header[i]}" = "$name" ]; then
      index=$i
      break
    fi
  done
  if [ $index -eq -1 ]; then
    return
  fi
  while IFS=, read -r -a row; do
    echo "${row[index]}"
  done
}