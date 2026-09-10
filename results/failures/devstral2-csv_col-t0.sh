csv_col() {
    local -n name="$1"
    local header
    read -r header
    IFS=, read -ra fields <<< "$header"
    local col_index=-1
    for i in "${!fields[@]}"; do
        if [[ "${fields[i]}" == "$name" ]]; then
            col_index=$i
            break
        fi
    done
    if [[ $col_index -eq -1 ]]; then
        return 1
    fi
    while IFS=, read -ra row; do
        if [[ $col_index -lt ${#row[@]} ]]; then
            echo "${row[col_index]}"
        fi
    done
}