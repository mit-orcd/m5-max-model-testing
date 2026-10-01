csv_col() {
    local col_name="$1"
    local col_num=$(head -n1 | tr ',' '\n' | grep -n "^$col_name$" | cut -d: -f1)
    if [ -z "$col_num" ]; then
        return 1
    fi
    cut -d',' -f"$col_num" | tail -n +2
}