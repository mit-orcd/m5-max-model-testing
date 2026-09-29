csv_col() {
    local -n name=$1
    local header
    local -a cols
    read -r header
    IFS=, read -ra cols <<< "$header"
    local col_index
    for i in "${!cols[@]}"; do
        if [[ "${cols[i]}" == "$name" ]]; then
            col_index=$i
            break
        fi
    done
    while IFS=, read -ra row; do
        echo "${row[col_index]}"
    done
}