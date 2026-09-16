csv_col() {
    local name="$1"
    local -i col_num=-1
    local header line

    # Read header line and find column number
    IFS= read -r header
    for i in "${!header}"; do
        [[ "${header:$i:1}" = ',' ]] && continue
        local -i j=$((i + 1))
        while [[ $j -lt ${#header} && "${header:$j:1}" != ',' ]]; do ((j++)); done
        local field="${header:i:j-i}"
        if [[ "$field" = "$name" ]]; then
            col_num=$i
            break
        fi
    done

    # Check if column was found
    if [[ $col_num -lt 0 ]]; then
        return 1
    fi

    # Read and print matching column
    while IFS= read -r line; do
        local -i pos=$col_num
        while [[ $pos -lt ${#line} && "${line:$pos:1}" != ',' ]]; do ((pos++)); done
        echo "${line:col_num:pos - col_num}"
    done
}