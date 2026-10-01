csv_col() {
    local name="$1"
    local col_num
    col_num=$(head -n1 | tr ',' '\n' | grep -nx "$name" | cut -d: -f1)
    if [[ -z "$col_num" ]]; then
        return 1
    fi
    tail -n +2 | awk -F',' -v col="$col_num" '{print $col}'
}