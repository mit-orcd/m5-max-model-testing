csv_col() {
    local name="$1"
    local header
    header=$(head -n 1)
    local col_num
    col_num=$(echo "$header" | awk -v name="$name" -F',' '{for(i=1;i<=NF;i++) if($i==name) print i}')
    if [[ -z "$col_num" ]]; then
        return 1
    fi
    tail -n +2 | awk -F',' -v col="$col_num" '{print $col}'
}