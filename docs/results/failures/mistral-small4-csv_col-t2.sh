csv_col() {
  local column_name="$1"
  local header
  header=$(head -1)
  local col_index
  col_index=$(echo "$header" | tr ',' '\n' | grep -n "^${column_name}$" | cut -d: -f1)
  if [ -z "$col_index" ]; then
    echo "Column not found: $column_name" >&2
    return 1
  fi
  tail -n +2 | cut -d, -f"$col_index"
}