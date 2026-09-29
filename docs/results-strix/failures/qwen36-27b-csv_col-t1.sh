csv_col() {
    local name="$1"
    local col_index
    col_index=$(head -n 1 | tr ',' '\n' | grep -n "^${name}$" | cut -d: -f1)
    if [[ -z "$col_index" ]]; then
        return 1
    fi
    tail -n +2 | awk -F',' -v col="$col_index" '{print $col}'
}