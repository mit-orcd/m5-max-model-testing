csv_col() {
    local name="$1"
    local header
    header=$(head -n 1)
    local col_index
    col_index=$(echo "$header" | awk -F',' -v col="$name" '{for(i=1;i<=NF;i++) if($i==col){print i; exit}}')
    if [ -z "$col_index" ]; then
        return 1
    fi
    tail -n +2 | awk -F',' -v idx="$col_index" '{print $idx}'
}