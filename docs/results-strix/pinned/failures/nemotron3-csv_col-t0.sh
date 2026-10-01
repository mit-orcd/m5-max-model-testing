csv_col() {
    local name="$1"
    local header_line
    header_line=$(head -n 1)
    local col_num=0
    local IFS=,
    read -ra headers <<< "$header_line"
    for i in "${!headers[@]}"; do
        if [[ "${headers[i]}" == "$name" ]]; then
            col_num=$((i + 1))
            break
        fi
    done
    if [[ $col_num -eq 0 ]]; then
        return 1
    fi
    tail -n +2 | cut -d, -f"$col_num"
}