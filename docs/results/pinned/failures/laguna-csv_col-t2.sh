csv_col() {
    local col_name="$1"
    local header_line=$(head -n 1)
    local col_num=0
    local found=0
    local IFS=','
    read -ra headers <<< "$header_line"
    for i in "${!headers[@]}"; do
        if [[ "${headers[$i]}" == "$col_name" ]]; then
            col_num=$((i + 1))
            found=1
            break
        fi
    done
    if [[ $found -eq 0 ]]; then
        return 1
    fi
    tail -n +2 | while IFS= read -r line; do
        if [[ -n "$line" ]]; then
            cut -d',' -f"$col_num" <<< "$line"
        fi
    done
}