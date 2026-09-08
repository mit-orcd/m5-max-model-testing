csv_col() {
    local name="$1"
    local col_idx
    
    # Read the header to find the column index
    IFS=',' read -r -a header
    for i in "${!header[@}"; do
        if [[ "${header[$i]}" == "$name" ]]; then
            col_idx=$((i + 1))
            break
        fi
    done

    if [[ -z "$col_idx" ]]; then
        return 1
    fi

    # Use awk to print the specific column from the remaining input
    awk -F',' -v col="$col_idx" '{print $col}'
}