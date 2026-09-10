csv_col() {
    local name="$1"
    local header line col_num
    IFS=',' read -ra header <<< "$(head -n 1)"
    for ((i=0; i<${#header[@]}; i++)); do
        if [[ "${header[$i]}" == "$name" ]]; then
            col_num=$i
            break
        fi
    done
    while IFS=',' read -ra line; do
        echo "${line[$col_num]}"
    done
}