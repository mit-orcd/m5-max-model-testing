csv_col() {
    local name="$1"
    local header
    header=$(head -n 1)
    local idx=0
    local col_idx=-1
    IFS=',' read -ra cols <<< "$header"
    for i in "${!cols[@]}"; do
        if [[ "${cols[$i]}" == "$name" ]]; then
            col_idx=$i
            break
        fi
    done
    if [[ $col_idx -eq -1 ]]; then
        return 1
    fi
    tail -n +2 | while IFS=',' read -ra row; do
        echo "${row[$col_idx]}"
    done
}