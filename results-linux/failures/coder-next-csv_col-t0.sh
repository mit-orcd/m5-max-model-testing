csv_col() {
    local name="$1"
    local col_num
    IFS=',' read -ra header < <(head -n1)
    for i in "${!header[@]}"; do
        if [[ "${header[$i]}" == "$name" ]]; then
            col_num=$((i + 1))
            break
        fi
    done
    if [[ -z "${col_num+x}" ]]; then
        return 1
    fi
    tail -n +2 | cut -d',' -f"$col_num"
}