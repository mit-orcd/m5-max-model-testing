csv_col() {
    local -n name="$1"
    local header
    read -r header
    local -a cols=($header)
    local col_index=-1
    for i in "${!cols[@]}"; do
        if [[ "${cols[i]}" == "$name" ]]; then
            col_index=$i
            break
        fi
    done
    if [[ $col_index -eq -1 ]]; then
        return 1
    fi
    while IFS=, read -r -a row; do
        if [[ $col_index -lt ${#row[@]} ]]; then
            echo "${row[col_index]}"
        fi
    done
}