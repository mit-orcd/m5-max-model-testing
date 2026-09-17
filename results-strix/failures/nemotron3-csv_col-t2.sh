csv_col() {
    local name="$1"
    local header_line
    header_line=$(head -n 1)
    local col_num
    col_num=$(echo "$header_line" | tr ',' '\n' | grep -n "^$name$" | cut -d: -f1)
    if [ -z "$col_num" ]; then
        return 1
    fi
    tail -n +2 | cut -d, -f"$col_num"
}