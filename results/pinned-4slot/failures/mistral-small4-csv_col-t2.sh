csv_col() {
  local name="$1"
  local header
  local col_index
  local line

  header=$(head -n 1)
  col_index=$(echo "$header" | awk -F, -v name="$name" '{
    for (i=1; i<=NF; i++) {
      if ($i == name) {
        print i;
        exit;
      }
    }
    exit 1;
  }')

  if [ -z "$col_index" ]; then
    echo "Column '$name' not found in CSV header" >&2
    exit 1
  fi

  while IFS= read -r line; do
    echo "$line" | awk -F, -v index="$col_index" '{
      if (index <= NF) {
        print $index;
      }
    }'
  done
}