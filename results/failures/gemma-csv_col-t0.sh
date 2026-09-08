csv_col() {
    local target_name="$1"
    local col_idx
    
    # Read header to find column index
    IFS=',' read -r -a header
    for i in "${!header[@]}"; do
        if [[ "${header[$i]}" == "$target_name" ]]; then
            col_idx=$((i + 1))
            break
        fi
    done

    if [[ -z "$col_idx" ]]; then
        return 1
    fi

    # Use awk to handle the extraction
    awk -F',' -v col="$col_idx" 'NR > 1 {print $col}'
}