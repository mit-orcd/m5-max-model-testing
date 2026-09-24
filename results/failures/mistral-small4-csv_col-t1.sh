csv_col() {
  local name="$1"
  local header
  header=$(head -n1 | tr ',' '\n')
  local colnum
  colnum=$(echo "$header" | grep -ni "^$name$" | cut -d: -f1)
  if [[ -z "$colnum" ]]; then
    return 1
  fi
  tail -n +2 | cut -d, -f"$colnum"
}