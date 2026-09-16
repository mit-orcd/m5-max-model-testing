csv_col() {
    local name="$1"
    local header_line=$(head -n 1)
    local col_num=0
    local found=0
    IFS=',' read -ra headers <<< "$header_line"
    for i in "${!headers[@]}"; do
        if [[ "${headers[$i]}" == "$name" ]]; then
            col_num=$((i + 1))
            found=1
            break
        fi
    done
    if [[ $found -eq 0 ]]; then
        echo "Column '$name' not found" >&2
        return 1
    fi
    tail -n +2 | cut -d',' -f"$col_num"
}