csv_col() {
  local name="$1"
  local header
  local colnum
  local line

  header=$(head -n 1)
  colnum=$(head -n 1 <<< "$header" | tr ',' '\n' | grep -n "^$name$" | head -n 1 | cut -d ':' -f 1)

  if [[ -z "$colnum" ]]; then
    echo "Column '$name' not found" >&2
    return 1
  fi

  while IFS= read -r line; do
    echo "$line" | cut -d ',' -f "$colnum" | tr -d '"'
  done
}